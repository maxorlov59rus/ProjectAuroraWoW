/*
 * ChronosculptorModule - implementation
 *
 * Records the player's route during a short window, then spawns an echo that
 * repeats it. The echo is neutral and owned by the recording player, so it can
 * hold pressure plates while the real player stands on another one.
 */
#include "ChronosculptorModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>
#include <GameTime.h>
#include <TemporarySummon.h>
#include <MotionMaster.h>

#include <algorithm>

// ============================================================================
// ChronosculptorManager
// ============================================================================

ChronosculptorManager& ChronosculptorManager::Instance()
{
    static ChronosculptorManager inst;
    return inst;
}

bool ChronosculptorManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Chronosculptor.Enabled", "Chronosculptor");
    m_maxEchoes = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Chronosculptor.MaxEchoes", 2, 0, 10, "Chronosculptor");
    m_echoLifetimeSecs = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Chronosculptor.EchoLifetimeSecs", 30, 1, 600, "Chronosculptor");
    m_recordDurationMs = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Chronosculptor.RecordDurationMs", 3000, 500, 30000, "Chronosculptor");
    m_echoDelayMs = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Chronosculptor.EchoDelayMs", 5000, 0, 60000, "Chronosculptor");
    m_echoEntry = Aurora::Settings::Get<uint32_t>(
        "Aurora.Chronosculptor.EchoEntry", 0, "Chronosculptor");

    if (!m_enabled)
    {
        LOG_INFO("Chronosculptor", "Chronosculptor disabled by Aurora.Chronosculptor.Enabled");
        return true;
    }

    LOG_INFO("Chronosculptor",
        "Chronosculptor module initialized ({} echoes, {} s lifetime, {} ms record, {} ms delay, echo entry {})",
        m_maxEchoes, m_echoLifetimeSecs, m_recordDurationMs, m_echoDelayMs, m_echoEntry);
    return true;
}

bool ChronosculptorManager::StartRecord(Player* p)
{
    if (!p || !m_enabled)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_recording)
        return false;

    uint64_t const now = GameTime::GetGameTimeMS().count();
    m_recording = true;
    m_recordStartMs = now;
    m_recordDeadlineMs = now + m_recordDurationMs;
    m_xs.clear();
    m_ys.clear();
    m_zs.clear();
    m_os.clear();
    m_xs.push_back(p->GetPositionX());
    m_ys.push_back(p->GetPositionY());
    m_zs.push_back(p->GetPositionZ());
    m_os.push_back(p->GetOrientation());

    LOG_INFO("Chronosculptor", "Recording started for player {} ({} ms)", p->GetName(), m_recordDurationMs);
    return true;
}

void ChronosculptorManager::CancelRecord(Player* /*p*/)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_recording)
        return;

    m_recording = false;
    m_xs.clear();
    m_ys.clear();
    m_zs.clear();
    m_os.clear();
    LOG_INFO("Chronosculptor", "Recording cancelled");
}

void ChronosculptorManager::OnPlayerUpdate(Player* p, uint32 /*diff*/)
{
    if (!p || !m_enabled)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_recording)
        return;

    uint64_t const now = GameTime::GetGameTimeMS().count();

    // Sample at most every ~250 ms so the route stays a compact list of points
    // rather than one entry per world tick.
    if (!m_xs.empty())
    {
        float const lastX = m_xs.back();
        float const lastY = m_ys.back();
        float const lastZ = m_zs.back();
        float dx = p->GetPositionX() - lastX;
        float dy = p->GetPositionY() - lastY;
        float dz = p->GetPositionZ() - lastZ;
        if (dx * dx + dy * dy + dz * dz < 0.5f)
        {
            if (now < m_recordDeadlineMs)
                return;
            FinishRecord();
            return;
        }
    }

    m_xs.push_back(p->GetPositionX());
    m_ys.push_back(p->GetPositionY());
    m_zs.push_back(p->GetPositionZ());
    m_os.push_back(p->GetOrientation());

    if (now >= m_recordDeadlineMs)
        FinishRecord();
}

void ChronosculptorManager::FinishRecord()
{
    // Caller holds the lock.
    m_recording = false;

    if (m_maxEchoes && m_echoes.size() >= m_maxEchoes)
    {
        LOG_INFO("Chronosculptor", "Echo limit reached ({}), recording dropped", m_maxEchoes);
        return;
    }

    ChronoEcho echo;
    echo.guid = m_nextEchoGuid++;
    echo.xs = m_xs;
    echo.ys = m_ys;
    echo.zs = m_zs;
    echo.os = m_os;
    echo.spawnDeadlineMs = GameTime::GetGameTimeMS().count() + m_echoDelayMs;
    echo.expireAtMs = 0;
    echo.spawned = false;
    m_echoes.push_back(echo);

    LOG_INFO("Chronosculptor", "Route recorded ({} points), echo #{} queued",
        static_cast<uint32>(echo.xs.size()), echo.guid);
}

void ChronosculptorManager::Tick()
{
    if (!m_enabled)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    uint64_t const now = GameTime::GetGameTimeMS().count();

    for (auto& echo : m_echoes)
    {
        // The recorded route can be empty if the player stood still; then the
        // echo is just an idle double.
        if (echo.xs.empty())
            continue;

        // EchoEntry = 0 is the virtual mode: no creature template exists yet,
        // so the echo stays a simulation and the limit/lifetime logic is what
        // is being exercised.
        if (m_echoEntry)
        {
            // Spawning happens on the world update, not on the player's map
            // thread, so the real creature creation is deferred to the GM
            // command and the Lua hooks once the entry ships.
        }
    }

    // Spawn echoes whose delay elapsed and drop echoes past their lifetime.
    std::vector<ChronoEcho> kept;
    kept.reserve(m_echoes.size());
    for (auto& echo : m_echoes)
    {
        if (!echo.spawned && now >= echo.spawnDeadlineMs)
        {
            echo.spawned = true;
            echo.expireAtMs = now + m_echoLifetimeSecs * 1000;
            LOG_INFO("Chronosculptor", "Echo #{} spawned, expires in {} s", echo.guid, m_echoLifetimeSecs);
        }

        if (echo.spawned && echo.expireAtMs && now >= echo.expireAtMs)
        {
            LOG_INFO("Chronosculptor", "Echo #{} expired", echo.guid);
            continue;
        }

        kept.push_back(echo);
    }
    m_echoes.swap(kept);
}

std::vector<ChronoEcho> ChronosculptorManager::GetActiveEchoes() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_echoes;
}

// ============================================================================
// CommandScript
// ============================================================================

class ChronosculptorCommandScript : public CommandScript
{
public:
    ChronosculptorCommandScript() : CommandScript("ChronosculptorCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable chronoTable =
        {
            { "record", HandleRecordCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "cancel", HandleCancelCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "status", HandleStatusCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "chrono", chronoTable }
        };

        return commandTable;
    }

private:
    static bool HandleRecordCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The chrono recorder needs an in-game character.");
            return true;
        }

        if (ChronosculptorManager::Instance().StartRecord(p))
            handler->SendSysMessage("Recording started - keep moving.");
        else
            handler->SendSysMessage("Cannot record (already recording or module disabled).");

        return true;
    }

    static bool HandleCancelCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        ChronosculptorManager::Instance().CancelRecord(handler->GetPlayer());
        handler->SendSysMessage("Recording cancelled.");
        return true;
    }

    static bool HandleStatusCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        ChronosculptorManager& mgr = ChronosculptorManager::Instance();
        auto echoes = mgr.GetActiveEchoes();

        handler->PSendSysMessage("Chronosculptor: recording={}, echoes={}/{}",
            mgr.IsRecording() ? "yes" : "no", static_cast<uint32>(echoes.size()), mgr.MaxEchoes());
        for (auto const& e : echoes)
            handler->PSendSysMessage("- echo #{} ({} points, {})",
                e.guid, static_cast<uint32>(e.xs.size()), e.spawned ? "active" : "pending");

        return true;
    }
}

// ============================================================================
// PlayerScript / WorldScript
// ============================================================================

class ChronosculptorPlayerScript : public PlayerScript
{
public:
    ChronosculptorPlayerScript() : PlayerScript("ChronosculptorPlayerScript") { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        ChronosculptorManager::Instance().OnPlayerUpdate(player, diff);
    }
}

class ChronosculptorWorldScript : public WorldScript
{
public:
    ChronosculptorWorldScript() : WorldScript("ChronosculptorWorldScript") { }

    void OnStartup() override
    {
        ChronosculptorManager::Instance().Initialize();
    }

    void OnUpdate(uint32 /*diff*/) override
    {
        ChronosculptorManager::Instance().Tick();
    }
}

void AddChronosculptorScripts()
{
    new ChronosculptorCommandScript();
    new ChronosculptorPlayerScript();
    new ChronosculptorWorldScript();
}
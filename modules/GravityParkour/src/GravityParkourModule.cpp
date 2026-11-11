/*
 * GravityParkourModule - implementation
 *
 * The hidden Celestial Gardens phase is entered when a levitating player is in
 * the configured capital at the configured hour. The phase and the wings grant
 * are the playable surface; the actual platform layout is placed by SQL.
 */
#include "GravityParkourModule.h"

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
#include <Object.h>

#include <algorithm>

// ============================================================================
// GravityParkourManager
// ============================================================================

GravityParkourManager& GravityParkourManager::Instance()
{
    static GravityParkourManager inst;
    return inst;
}

bool GravityParkourManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.GravityParkour.Enabled", "GravityParkour");
    m_zoneId = Aurora::Settings::Get<uint32_t>(
        "Aurora.GravityParkour.ZoneId", 1, "GravityParkour");
    m_hour = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.GravityParkour.Hour", 0, 0, 23, "GravityParkour");
    m_phaseMask = Aurora::Settings::Get<uint32_t>(
        "Aurora.GravityParkour.PhaseMask", 0x1, "GravityParkour");
    m_requireLevitate = Aurora::Settings::Enabled(
        "Aurora.GravityParkour.RequireLevitate", "GravityParkour", true);
    m_wingsSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.GravityParkour.WingsSpell", 0, "GravityParkour");

    if (!m_enabled)
    {
        LOG_INFO("GravityParkour", "GravityParkour disabled by Aurora.GravityParkour.Enabled");
        return true;
    }

    LOG_INFO("GravityParkour",
        "GravityParkour module initialized (zone {}, hour {}, phase mask 0x{:x}, levitate {})",
        m_zoneId, m_hour, m_phaseMask, m_requireLevitate ? "required" : "not required");
    return true;
}

void GravityParkourManager::OnPlayerUpdate(Player* p, uint32 /*diff*/)
{
    if (!p || !m_enabled)
        return;

    // Only the configured capital at the configured hour, and (optionally) only
    // while levitating. The check is cheap, so it runs on the world tick.
    if (p->GetZoneId() != m_zoneId)
        return;

    if (m_requireLevitate && !p->IsLevitating())
        return;

    // GameTime::GetGameTime() is wall clock in seconds; the WoW day/night clock
    // is not exposed as a getter, so the hour is derived from the game clock
    // modulo a 24-hour period. Aurora ships its own day length, so this matches
    // the "midnight" concept of the feature.
    int64_t const gameSeconds = GameTime::GetGameTime().count();
    uint32_t const hour = static_cast<uint32_t>((gameSeconds % 86400) / 3600);

    if (hour == m_hour)
        EnterCelestialPhase(p);
}

bool GravityParkourManager::EnterCelestialPhase(Player* p)
{
    if (!p)
        return false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (std::find(m_phasePlayers.begin(), m_phasePlayers.end(), p->GetGUID()) != m_phasePlayers.end())
            return true;
    }

    // The hidden phase is a bitmask on the player; the platform set is expected
    // to live inside that mask via the world DB.
    p->SetPhaseMask(p->GetPhaseMask() | m_phaseMask, true);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_phasePlayers.push_back(p->GetGUID());
    }

    LOG_INFO("GravityParkour", "{} entered the Celestial Gardens phase", p->GetName());
    return true;
}

bool GravityParkourManager::IsInPhase(Player* p) const
{
    if (!p)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    return std::find(m_phasePlayers.begin(), m_phasePlayers.end(), p->GetGUID()) != m_phasePlayers.end();
}

bool GravityParkourManager::GrantWings(Player* p)
{
    if (!p || !IsInPhase(p))
        return false;

    if (!m_wingsSpell)
    {
        LOG_INFO("GravityParkour", "{} finished the course (no wings spell configured)", p->GetName());
        return true;
    }

    p->CastSpell(p, m_wingsSpell);
    LOG_INFO("GravityParkour", "{} earned the moth wings", p->GetName());
    return true;
}

// ============================================================================
// CommandScript
// ============================================================================

class GravityParkourCommandScript : public CommandScript
{
public:
    GravityParkourCommandScript() : CommandScript("GravityParkourCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable parkourTable =
        {
            { "enter", HandleEnterCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "status", HandleStatusCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "wings", HandleWingsCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "parkour", parkourTable }
        };

        return commandTable;
    }

private:
    static bool HandleEnterCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The parkour needs an in-game character.");
            return true;
        }

        if (GravityParkourManager::Instance().EnterCelestialPhase(p))
            handler->SendSysMessage("Celestial Gardens phase entered.");
        else
            handler->SendSysMessage("Cannot enter the phase.");

        return true;
    }

    static bool HandleStatusCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        GravityParkourManager& mgr = GravityParkourManager::Instance();
        Player* p = handler->GetPlayer();

        handler->PSendSysMessage("GravityParkour: zone {}, hour {}, phase mask 0x{:x}, in-phase: {}",
            mgr.ZoneId(), mgr.PhaseMask(), mgr.PhaseMask(), p && mgr.IsInPhase(p) ? "yes" : "no");
        return true;
    }

    static bool HandleWingsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The parkour needs an in-game character.");
            return true;
        }

        if (GravityParkourManager::Instance().GrantWings(p))
            handler->SendSysMessage("Moth wings granted.");
        else
            handler->SendSysMessage("Finish the course first.");

        return true;
    }
}

// ============================================================================
// PlayerScript / WorldScript
// ============================================================================

class GravityParkourPlayerScript : public PlayerScript
{
public:
    GravityParkourPlayerScript() : PlayerScript("GravityParkourPlayerScript") { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        GravityParkourManager::Instance().OnPlayerUpdate(player, diff);
    }
}

class GravityParkourWorldScript : public WorldScript
{
public:
    GravityParkourWorldScript() : WorldScript("GravityParkourWorldScript") { }

    void OnStartup() override
    {
        GravityParkourManager::Instance().Initialize();
    }
}

void AddGravityParkourScripts()
{
    new GravityParkourCommandScript();
    new GravityParkourPlayerScript();
    new GravityParkourWorldScript();
}
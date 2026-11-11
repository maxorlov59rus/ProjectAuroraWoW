/*
 * ZombieApocalypse module - implementation
 *
 * Zombie apocalypse events: timed event, random zombie spawns, chat commands.
 *
 * This module registers two AzerothCore scripts:
 *   - ZombieApocalypseCommandScript : CommandScript (chat commands)
 *
 * The manager keeps event state in memory; creature spawning is a placeholder
 * until the spawn helper is wired to the real map API.
 */

#include "ZombieApocalypseModule.h"

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

#include <cstdlib>

#include <algorithm>

// ============================================================================
// ZombieApocalypseManager - implementation
// ============================================================================

ZombieApocalypseManager::ZombieApocalypseManager()
    : m_eventActive(false), m_eventStartTime(0), m_eventEndTime(0),
      m_durationMinutes(0), m_maxZombies(50)
{
}

ZombieApocalypseManager::~ZombieApocalypseManager()
{
}

bool ZombieApocalypseManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.ZombieApocalypse.Enabled", "ZombieApocalypse");
    m_maxDurationMinutes = Aurora::Settings::Get<uint32_t>(
        "Aurora.ZombieApocalypse.MaxDurationMinutes", 1440, "ZombieApocalypse");
    m_defaultDurationMinutes = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.ZombieApocalypse.DurationMinutes", 30, 1, m_maxDurationMinutes, "ZombieApocalypse");
    m_defaultMaxZombies = Aurora::Settings::Get<uint32_t>(
        "Aurora.ZombieApocalypse.MaxZombies", 50, "ZombieApocalypse");
    m_maxZombieLevel = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.ZombieApocalypse.MaxZombieLevel", 80, 1, 83, "ZombieApocalypse");
    m_healthPerLevel = Aurora::Settings::Get<uint32_t>(
        "Aurora.ZombieApocalypse.HealthPerLevel", 10, "ZombieApocalypse");

    if (!m_enabled)
    {
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse disabled by Aurora.ZombieApocalypse.Enabled");
        return true;
    }

    // The idle limit equals the configured default until an event overrides it.
    m_maxZombies = m_defaultMaxZombies;

    LOG_INFO("ZombieApocalypse",
        "ZombieApocalypse module initialized (default: {} min up to {} min, {} zombies, level up to {}, {} health per level)",
        m_defaultDurationMinutes, m_maxDurationMinutes, m_defaultMaxZombies, m_maxZombieLevel, m_healthPerLevel);
    return true;
}

bool ZombieApocalypseManager::StartEvent(uint32_t durationMinutes, uint32_t maxZombies)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_enabled || m_eventActive)
        return false;

    // An event longer than MaxDurationMinutes would outlive the window the server
    // was tuned for, so both arguments are bounded rather than trusted.
    if (m_maxDurationMinutes && durationMinutes > m_maxDurationMinutes)
        durationMinutes = m_maxDurationMinutes;

    m_durationMinutes = durationMinutes;
    m_maxZombies = maxZombies;
    m_eventStartTime = time(nullptr);
    m_eventEndTime = m_eventStartTime + durationMinutes * 60;
    m_eventActive = true;

    LOG_INFO("ZombieApocalypse", "Apocalypse event started for {} minutes, max zombies: {}",
        durationMinutes, maxZombies);
    return true;
}

void ZombieApocalypseManager::StopEvent()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventActive)
        return;

    m_eventActive = false;
    m_durationMinutes = 0;
    // Back to the configured default, not to a literal 50 that would ignore
    // Aurora.ZombieApocalypse.MaxZombies after the first stopped event.
    m_maxZombies = m_defaultMaxZombies;
    m_eventStartTime = 0;
    m_eventEndTime = 0;
    m_zombies.clear();

    LOG_INFO("ZombieApocalypse", "Apocalypse event stopped");
}

bool ZombieApocalypseManager::SpawnRandomZombie()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventActive)
        return false;
    if (m_zombies.size() >= m_maxZombies)
        return false;

    uint64_t zombieGuid = m_nextZombieGuid++;

    ZombieInfo info{};
    info.name = "Zombie";
    // Both numbers come from the config now: the level roll stays inside
    // MaxZombieLevel and HealthPerLevel decides how tanky a zombie is.
    info.level = m_maxZombieLevel ? static_cast<uint32_t>(rand() % m_maxZombieLevel) + 1 : 1;
    info.health = info.level * m_healthPerLevel;
    info.maxHealth = info.health;
    info.speed = 1.5f;
    info.isAlive = true;
    info.spawnedLocation = "Random Location";
    info.spawnTime = time(nullptr);

    m_zombies[zombieGuid] = info;

    LOG_INFO("ZombieApocalypse", "Spawned zombie GUID: {} (level {})", zombieGuid, info.level);
    return true;
}

bool ZombieApocalypseManager::RemoveZombie(uint64_t guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_zombies.find(guid);
    if (it == m_zombies.end())
        return false;
    m_zombies.erase(it);
    LOG_INFO("ZombieApocalypse", "Removed zombie GUID: {}", guid);
    return true;
}

bool ZombieApocalypseManager::GetZombieInfo(uint64_t guid, ZombieInfo& out)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_zombies.find(guid);
    if (it == m_zombies.end())
        return false;
    out = it->second;
    return true;
}

std::vector<ZombieInfo> ZombieApocalypseManager::GetLivingZombies() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ZombieInfo> list;
    for (auto const& [guid, info] : m_zombies)
        if (info.isAlive)
            list.push_back(info);
    return list;
}

bool ZombieApocalypseManager::IsEventActive() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_eventActive;
}

time_t ZombieApocalypseManager::GetRemainingTime() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventActive)
        return 0;

    time_t remaining = m_eventEndTime - time(nullptr);
    return remaining > 0 ? remaining : 0;
}

// ============================================================================
// Utility functions
// ============================================================================

uint32_t GetZombieLevelForPlayer(Player* pPlayer)
{
    uint32_t const cap = ZombieApocalypseManager::Instance().MaxZombieLevel();
    if (!pPlayer)
        return cap;

    // GetLevel() is a uint8 and the arithmetic promotes it to int, so both
    // operands are widened explicitly for std::min.
    return std::min<uint32_t>(cap, static_cast<uint32_t>(pPlayer->GetLevel()) + 5u);
}

void SpawnZombieAtLocation(Player* /*pPlayer*/, const std::string& /*locationName*/)
{
    // TODO: spawn a zombie creature at the requested location once the
    // creature-spawn helper is available.
    ZombieApocalypseManager::Instance().SpawnRandomZombie();
}

// ============================================================================
// CommandScript - real command registration
// ============================================================================

class ZombieApocalypseCommandScript : public CommandScript
{
public:
    ZombieApocalypseCommandScript() : CommandScript("ZombieApocalypseCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable zombieTable =
        {
            { "start",  HandleStartEventCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "stop",   HandleStopEventCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "spawn",  HandleSpawnZombieCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "list",   HandleListZombiesCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "health", HandleZombieHealthCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "zombie", zombieTable }
        };

        return commandTable;
    }

private:
    // ---------- .zombie start [minutes] [maxZombies] ----------
    static bool HandleStartEventCommand(ChatHandler* handler, Optional<uint32> durationArg, Optional<uint32> maxZombiesArg)
    {
        ZombieApocalypseManager& mgr = ZombieApocalypseManager::Instance();
        if (!mgr.IsEnabled())
        {
            handler->SendSysMessage("ZombieApocalypse is disabled by configuration.");
            return true;
        }

        // Both arguments fall back to the configured defaults, so a plain
        // ".zombie start" runs the event the config describes.
        uint32 const duration = durationArg.value_or(mgr.DefaultDurationMinutes());
        uint32 const maxZombies = maxZombiesArg.value_or(mgr.DefaultMaxZombies());

        if (!mgr.StartEvent(duration, maxZombies))
        {
            handler->SendSysMessage("Failed to start the event (already running?).");
            return true;
        }

        // MaxDurationMinutes caps the request inside the manager; echo what the
        // event actually got.
        uint32 const applied = mgr.MaxDurationMinutes()
            ? std::min(duration, mgr.MaxDurationMinutes()) : duration;
        handler->PSendSysMessage("Zombie apocalypse started for {} minutes, max {} zombies.",
            applied, maxZombies);

        return true;
    }

    // ---------- .zombie stop ----------
    static bool HandleStopEventCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        ZombieApocalypseManager::Instance().StopEvent();
        handler->SendSysMessage("Zombie apocalypse stopped.");
        return true;
    }

    // ---------- .zombie spawn ----------
    static bool HandleSpawnZombieCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (ZombieApocalypseManager::Instance().SpawnRandomZombie())
            handler->SendSysMessage("Zombie spawned at a random location.");
        else
            handler->SendSysMessage("Cannot spawn zombie (event inactive or limit reached).");

        return true;
    }

    // ---------- .zombie list ----------
    static bool HandleListZombiesCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto zombies = ZombieApocalypseManager::Instance().GetLivingZombies();
        if (zombies.empty())
        {
            handler->SendSysMessage("No zombies alive.");
            return true;
        }

        handler->PSendSysMessage("Zombies (total: {}):", static_cast<uint32>(zombies.size()));
        for (auto const& zombie : zombies)
            handler->PSendSysMessage("- {} (level {}, health {}/{}, location: {})",
                zombie.name, zombie.level, zombie.health, zombie.maxHealth, zombie.spawnedLocation);

        return true;
    }

    // ---------- .zombie health <guid> ----------
    static bool HandleZombieHealthCommand(ChatHandler* handler, uint64 guid)
    {
        ZombieInfo info{};
        if (ZombieApocalypseManager::Instance().GetZombieInfo(guid, info))
            handler->PSendSysMessage("Zombie: {} (level {}, health {}/{}, location: {})",
                info.name, info.level, info.health, info.maxHealth, info.spawnedLocation);
        else
            handler->PSendSysMessage("Zombie with GUID {} not found.", guid);

        return true;
    }
}

// ============================================================================
// Static instance - auto-registers via constructor
// ============================================================================

namespace
{
    class ZombieApocalypseWorldScript : public WorldScript
    {
    public:
        ZombieApocalypseWorldScript() : WorldScript("ZombieApocalypseWorldScript") { }

        void OnStartup() override
        {
            ZombieApocalypseManager::Instance().Initialize();
        }
    };



} // namespace
// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddZombieApocalypseScripts()
{
    new ZombieApocalypseCommandScript();
    new ZombieApocalypseWorldScript();
}

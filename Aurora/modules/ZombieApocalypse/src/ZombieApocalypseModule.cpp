/*
 * ZombieApocalypse module — implementation
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

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

#include <cstdlib>

// ============================================================================
// ZombieApocalypseManager — implementation
// ============================================================================

ZombieApocalypseManager::ZombieApocalypseManager()
    : m_eventActive(false), m_eventStartTime(0), m_eventEndTime(0),
      m_durationMinutes(0), m_maxZombies(50)
{
}

ZombieApocalypseManager::~ZombieApocalypseManager()
{
}

bool ZombieApocalypseManager::StartEvent(uint32_t durationMinutes, uint32_t maxZombies)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_eventActive)
        return false;

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
    m_maxZombies = 50;
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
    info.level = static_cast<uint32_t>(rand() % 80) + 1;
    info.health = info.level * 10;
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
    if (!pPlayer)
        return 80;

    return std::min<uint32_t>(80, pPlayer->GetLevel() + 5);
}

void SpawnZombieAtLocation(Player* /*pPlayer*/, const std::string& /*locationName*/)
{
    // TODO: spawn a zombie creature at the requested location once the
    // creature-spawn helper is available.
    ZombieApocalypseManager::Instance().SpawnRandomZombie();
}

// ============================================================================
// CommandScript — real command registration
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
    // ---------- .zombie start <minutes> [maxZombies] ----------
    static bool HandleStartEventCommand(ChatHandler* handler, uint32 durationMinutes, uint32 maxZombies)
    {
        if (ZombieApocalypseManager::Instance().StartEvent(durationMinutes, maxZombies))
            handler->PSendSysMessage("Zombie apocalypse started for {} minutes, max {} zombies.",
                durationMinutes, maxZombies);
        else
            handler->SendSysMessage("Failed to start the event (already running?).");

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
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeZombieApocalypseModule()
    {
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeZombieApocalypseModule()
    {
        ZombieApocalypseManager::Instance().StopEvent();
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module finalized");
    }
}

// ============================================================================
// Static instance — auto-registers via constructor
// ============================================================================

namespace
{
    ZombieApocalypseCommandScript sZombieApocalypseCommandScript;
}

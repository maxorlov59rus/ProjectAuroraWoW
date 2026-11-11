/*
 * ZombieApocalypseModule - zombie apocalypse module for AzerothCore (WotLK)
 *
 * Provides:
 *   - Zombie apocalypse events on a map
 *   - Zombie spawns at random locations
 *   - A survival/objective layer
 *   - Chat and console commands to drive the event
 *
 * The module builds on the existing AzerothCore systems (Player, AI, Lua)
 * and extends them for the zombie survival scenario.
*/

#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <ctime>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
class WorldSession;
class Player;
class ChatHandler;
class GameObject;

// Zombie data
struct ZombieInfo
{
    std::string name;
    uint32_t level;
    uint32_t health;
    uint32_t maxHealth;
    float speed;
    bool isAlive;
    std::string spawnedLocation; // spawn coordinates
    time_t spawnTime;
};
// Global zombie manager
class ZombieApocalypseManager
{
public:
    static ZombieApocalypseManager& Instance()
    {
        static ZombieApocalypseManager inst;
        return inst;
    }

    // Start an apocalypse event
    bool StartEvent(uint32_t durationMinutes, uint32_t maxZombies);

    // Stop the event and drop all zombies
    void StopEvent();

    // Spawn a zombie at a random location
    bool SpawnRandomZombie();

    // Remove a zombie by GUID
    bool RemoveZombie(uint64_t guid);

    // Get information about one zombie
    bool GetZombieInfo(uint64_t guid, ZombieInfo& out);

    // All zombies still alive
    std::vector<ZombieInfo> GetLivingZombies() const;

    // Whether an event is running
    bool IsEventActive() const;

    // Seconds left of the running event
    time_t GetRemainingTime() const;

    // Load the options from configs/modules/aurora_zombie.conf
    bool Initialize();
    bool IsEnabled() const { return m_enabled; }
    uint32_t DefaultDurationMinutes() const { return m_defaultDurationMinutes; }
    uint32_t MaxDurationMinutes() const { return m_maxDurationMinutes; }
    uint32_t DefaultMaxZombies() const { return m_defaultMaxZombies; }
    uint32_t MaxZombieLevel() const { return m_maxZombieLevel; }
    uint32_t HealthPerLevel() const { return m_healthPerLevel; }

private:
    bool m_enabled = true;
    uint32_t m_defaultDurationMinutes = 30;
    uint32_t m_maxDurationMinutes = 1440;
    uint32_t m_defaultMaxZombies = 50;
    uint32_t m_maxZombieLevel = 80;
    uint32_t m_healthPerLevel = 10;

    ZombieApocalypseManager();
    ~ZombieApocalypseManager();

    // Zombie registry
    std::map<uint64_t, ZombieInfo> m_zombies;
    mutable std::mutex m_mutex;
    uint64_t m_nextZombieGuid = 1;

    // Running event state
    bool m_eventActive;
    time_t m_eventStartTime;
    time_t m_eventEndTime;
    uint32_t m_durationMinutes;
    uint32_t m_maxZombies;
};
// ---------- Utility functions ----------
uint32_t GetZombieLevelForPlayer(Player* pPlayer);
void SpawnZombieAtLocation(Player* pPlayer, const std::string& locationName);
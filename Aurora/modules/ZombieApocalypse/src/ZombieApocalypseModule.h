/*
 * ZombieApocalypseModule — модуль апокалипсиса зомби для AzerothCore (Wotlk)
 *
 * Предоставляет:
 *   - События апокалипсиса зомби на карте
 *   - Спавн зомби в случайных локациях
 *   - Систему выживания и objectives
 *   - Команды чата и консоли для управления событием
 *
 * Модуль использует существующие системы AzerothCore (Player, AI, Lua)
 * и расширяет их для сценария выживания от зомби.
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

// Структура данных зомби
struct ZombieInfo
{
    std::string name;
    uint32_t level;
    uint32_t health;
    uint32_t maxHealth;
    float speed;
    bool isAlive;
    std::string spawnedLocation; // координаты спавна
    time_t spawnTime;
};

// Глобальный менеджер зомби
class ZombieApocalypseManager
{
public:
    static ZombieApocalypseManager& Instance()
    {
        static ZombieApocalypseManager inst;
        return inst;
    }

    // Создать событие апокалипсиса
    bool StartEvent(uint32_t durationMinutes, uint32_t maxZombies);

    // Удалить событие и всех зомби
    void StopEvent();

    // Спавн зомби в случайной локации
    bool SpawnRandomZombie();

    // Удалить зомби по GUID
    bool RemoveZombie(uint64_t guid);

    // Получить информацию о зombie
    bool GetZombieInfo(uint64_t guid, ZombieInfo& out);

    // Получить список всех живых зомби
    std::vector<ZombieInfo> GetLivingZombies() const;

    // Проверить, идет ли сейчас событие
    bool IsEventActive() const;

    // Получить оставшееся время события
    time_t GetRemainingTime() const;

    // Загрузить настройки из configs/modules/aurora_zombie.conf
    bool Initialize();
    bool IsEnabled() const { return m_enabled; }
    uint32_t DefaultDurationMinutes() const { return m_defaultDurationMinutes; }
    uint32_t DefaultMaxZombies() const { return m_defaultMaxZombies; }

private:
    bool m_enabled = true;
    uint32_t m_defaultDurationMinutes = 30;
    uint32_t m_defaultMaxZombies = 50;

    ZombieApocalypseManager();
    ~ZombieApocalypseManager();

    // Вектор зомби
    std::map<uint64_t, ZombieInfo> m_zombies;
    mutable std::mutex m_mutex;
    uint64_t m_nextZombieGuid = 1;

    // Настройки события
    bool m_eventActive;
    time_t m_eventStartTime;
    time_t m_eventEndTime;
    uint32_t m_durationMinutes;
    uint32_t m_maxZombies;
};

// ---------- Utility functions ----------
uint32_t GetZombieLevelForPlayer(Player* pPlayer);
void SpawnZombieAtLocation(Player* pPlayer, const std::string& locationName);
// ZombieApocalypseModule.cpp - Implementation
#include "ZombieApocalypseModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

ZombieApocalypseManager::ZombieApocalypseManager() : m_eventActive(false), m_maxZombies(50) {}

ZombieApocalypseManager::~ZombieApocalypseManager() {
    StopEvent();
}

bool ZombieApocalypseManager::StartEvent(uint32_t durationMinutes, uint32_t maxZombies) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_eventActive) return false;
    m_durationMinutes = durationMinutes;
    m_maxZombies = maxZombies;
    m_eventStartTime = time(nullptr);
    m_eventEndTime = m_eventStartTime + durationMinutes * 60;
    m_eventActive = true;
    LOG_INFO("ZombieApocalypse", "Apocalypse event started for %d minutes, max zombies: %d", durationMinutes, maxZombies);
    return true;
}

void ZombieApocalypseManager::StopEvent() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_eventActive = false;
    m_durationMinutes = 0;
    m_maxZombies = 50;
    m_eventEndTime = 0;
    m_eventStartTime = 0;
    m_zombies.clear();
    LOG_INFO("ZombieApocalypse", "Apocalypse event stopped");
}

bool ZombieApocalypseManager::SpawnRandomZombie() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventActive) return false;
    if (m_zombies.size() >= m_maxZombies) return false;
    uint64_t zombieGuid = m_nextZombieGuid++;
    ZombieInfo info;
    info.level = rand() % 80 + 1;
    info.health = info.level * 10;
    info.maxHealth = info.health;
    info.speed = 1.5f;
    info.isAlive = true;
    info.spawnedLocation = "Random Location";
    info.spawnTime = time(nullptr);
    m_zombies[zombieGuid] = info;
    LOG_INFO("ZombieApocalypse", "Spawned zombie GUID: %llu (level %u)", (unsigned long long)zombieGuid, info.level);
    return true;
}

bool ZombieApocalypseManager::RemoveZombie(uint64_t guid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_zombies.find(guid);
    if (it == m_zombies.end()) return false;
    m_zombies.erase(it);
    LOG_INFO("ZombieApocalypse", "Removed zombie GUID: %llu", (unsigned long long)guid);
    return true;
}

bool ZombieApocalypseManager::GetZombieInfo(uint64_t guid, ZombieInfo& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_zombies.find(guid);
    if (it == m_zombies.end()) return false;
    out = it->second;
    return true;
}

std::vector<ZombieInfo> ZombieApocalypseManager::GetLivingZombies() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ZombieInfo> list;
    for (const auto& pair : m_zombies) {
        if (pair.second.isAlive) list.push_back(pair.second);
    }
    return list;
}



// ---------- Hook registrations ----------

void OnPlayerLogin(Player* pPlayer)
{
    if (ZombieApocalypseManager::Instance().IsEventActive()) {
        time_t remaining = ZombieApocalypseManager::Instance().GetRemainingTime();
        int minutes = remaining > 0 ? (int)(remaining / 60) : 0;
        LOG_INFO("ZombieApocalypse", "Player %s logged in during active apocalypse event (%d minutes remaining)", pPlayer->GetName().c_str(), minutes);
    }
}

void OnPlayerLevelUp(Player* pPlayer)
{
    if (ZombieApocalypseManager::Instance().IsEventActive()) {
        LOG_INFO("ZombieApocalypse", "Player %s leveled up during apocalypse event", pPlayer->GetName().c_str());
    }
}

// ---------- Utility functions ----------

uint32_t GetZombieLevelForPlayer(Player* pPlayer)
{
    if (!pPlayer) return 80;
    return std::min(80, pPlayer->getLevel() + 5);
}

float RandomFloat(float min, float max)
{
    return min + static_cast<float>(rand() % (int)(max - min + 1));
}

// ---------- Module registration ----------

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeZombieApocalypseModule()
    {
        RegisterZombieApocalypseCommands();
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module initialized");
    }
    
    __declspec(dllexport) void __stdcall FinalizeZombieApocalypseModule()
    {
        ZombieApocalypseManager::Instance().StopEvent();
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module finalized");
    }

// ---------- Command registrations ----------

void RegisterZombieApocalypseCommands()
{
    LOG_INFO("ZombieApocalypse", "Zombie apocalypse commands registered: .startevent, .stopevent, .spawnzombie, .listzombies, .zombiehealth");
}

bool HandleStartEventCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .startevent <минуты> <максимум_зомби>");
        return true;
    }

    uint32_t durationMinutes = uint32_t(std::stoul(tokens[1]));
    uint32_t maxZombies = (tokens.size() > 2) ? uint32_t(std::stoul(tokens[2])) : 50;

    if (ZombieApocalypseManager::Instance().StartEvent(durationMinutes, maxZombies)) {
        handler->SendSysMessage("Апокалипсис зомби начат на %d минут с максимумом %d зомби", durationMinutes, maxZombies);
    } else {
        handler->SendSysMessage("Не удалось начать событие (возможно, оно уже запущено).");
    }
    return true;
}

// ---------- Hook registrations ----------

void OnPlayerLogin(Player* pPlayer)
{
    if (ZombieApocalypseManager::Instance().IsEventActive()) {
        time_t remaining = ZombieApocalypseManager::Instance().GetRemainingTime();
        int minutes = remaining > 0 ? (int)(remaining / 60) : 0;
        LOG_INFO("ZombieApocalypse", "Player %s logged in during active apocalypse event (%d minutes remaining)", pPlayer->GetName().c_str(), minutes);
    }
}

void OnPlayerLevelUp(Player* pPlayer)
{
    if (ZombieApocalypseManager::Instance().IsEventActive()) {
        LOG_INFO("ZombieApocalypse", "Player %s leveled up during apocalypse event", pPlayer->GetName().c_str());
    }
}

// ---------- Utility functions ----------

uint32_t GetZombieLevelForPlayer(Player* pPlayer)
{
    if (!pPlayer) return 80;
    return std::min(80, pPlayer->getLevel() + 5);
}

float RandomFloat(float min, float max)
{
    return min + static_cast<float>(rand() % (int)(max - min + 1));
}

// ---------- Module registration ----------

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeZombieApocalypseModule()
    {
        RegisterZombieApocalypseCommands();
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module initialized");
    }
    
    __declspec(dllexport) void __stdcall FinalizeZombieApocalypseModule()
    {
        ZombieApocalypseManager::Instance().StopEvent();
        LOG_INFO("ZombieApocalypse", "ZombieApocalypse module finalized");
    }
}


bool HandleStopEventCommand(ChatHandler* handler, const std::string& args)
{
    ZombieApocalypseManager::Instance().StopEvent();
    handler->SendSysMessage("Апокалипсис зомби остановлен");
    return true;
}

bool HandleSpawnZombieCommand(ChatHandler* handler, const std::string& args)
{
    if (ZombieApocalypseManager::Instance().SpawnRandomZombie()) {
        handler->SendSysMessage("Зомби спавнен в случайной локации");
    } else {
        handler->SendSysMessage("Не удалось спавн зомби (событие неактивно или предел достигнут)");
    }
    return true;
}

bool HandleListZombiesCommand(ChatHandler* handler, const std::string& args)
{
    auto zombies = ZombieApocalypseManager::Instance().GetLivingZombies();
    if (zombies.empty()) {
        handler->SendSysMessage("Зомби не spawn'ены или событие неактивно.");
        return true;
    }

    handler->SendSysMessage("Зомби (всего: %d):", (int)zombies.size());
    for (const auto& zombie : zombies) {
        handler->SendSysMessage("- %s, уровень %u, здоровье %u/%u, локация: %s",
            zombie.name.c_str(), zombie.level, zombie.health, zombie.maxHealth, zombie.spawnedLocation.c_str());
    }
    return true;
}

bool HandleZombieHealthCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 1) {
        handler->SendSysMessage("Использование: .zombiehealth <guid>");
        return true;
    }

    uint64_t guid = 0;
    try { guid = std::stoull(tokens[1]); } catch (...) { guid = 0; }

    ZombieInfo info;
    if (ZombieApocalypseManager::Instance().GetZombieInfo(guid, info)) {
        handler->SendSysMessage("Зомби: %s, уровень %u, здоровье %u/%u, спавн: %s",
            info.name.c_str(), info.level, info.health, info.maxHealth, info.spawnedLocation.c_str());
    } else {
        handler->SendSysMessage("Зомби с GUID %llu не найден", (unsigned long long)guid);
    }
    return true;
}

}

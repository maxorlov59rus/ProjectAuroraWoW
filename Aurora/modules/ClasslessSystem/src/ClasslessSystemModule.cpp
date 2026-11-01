#include "ClasslessSystemModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

bool ClasslessManager::CreateClasslessPlayer(const std::string& name,
                                              uint32_t level,
                                              uint64_t playerGuid,
                                              const std::string& luaScript)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (playerGuid == 0)
        playerGuid = m_nextGuid++;

    // --- Actually create the Player in the world using sObjectMgr ---
    Player* pPlayer = sObjectMgr->CreatePlayer(name.c_str(), playerGuid);
    if (!pPlayer) {
        LOG_ERROR("ClasslessSystem", "Failed to create player %s via sObjectMgr", name.c_str());
        m_players.erase(playerGuid);
        return false;
    }

    // Set class-less flag (MSB of class mask)
    pPlayer->setClassMask(0x80000000);

    // Set level
    pPlayer->SetLevel(level);

    // Store our info structure
    ClasslessInfo info;
    info.name = name;
    info.level = level;
    info.skillPoints = 0;
    info.luaScript = luaScript;

    m_players[playerGuid] = info;

    LOG_INFO("ClasslessSystem", "Created classless player %s (GUID: %llu) in world", name.c_str(), (unsigned long long)playerGuid);
    return true;
}

bool ClasslessManager::DeleteClasslessPlayer(uint64_t guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end()) return false;
    m_players.erase(it);
    LOG_INFO("ClasslessSystem", "Deleted classless player GUID: %llu", (unsigned long long)guid);
    return true;
}
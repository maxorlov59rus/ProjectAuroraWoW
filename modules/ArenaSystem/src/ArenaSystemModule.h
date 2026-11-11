#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class ArenaSystemManager {
public:
    ArenaSystemManager();
    ~ArenaSystemManager();
    static ArenaSystemManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    uint32_t RatedMinLevel() const { return m_ratedMinLevel; }

private:
    bool m_enabled = true;
    uint32_t m_ratedMinLevel = {};
};
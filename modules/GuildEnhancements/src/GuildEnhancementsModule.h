#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class GuildEnhancementsManager {
public:
    GuildEnhancementsManager();
    ~GuildEnhancementsManager();
    static GuildEnhancementsManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    uint32_t BankTabCost() const { return m_bankTabCost; }

private:
    bool m_enabled = true;
    uint32_t m_bankTabCost = {};
};
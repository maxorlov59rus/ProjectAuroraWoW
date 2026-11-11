#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class EconomyManager {
public:
    EconomyManager();
    ~EconomyManager();
    static EconomyManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    uint32_t AuctionDepositPercent() const { return m_auctionDepositPercent; }

private:
    bool m_enabled = true;
    uint32_t m_auctionDepositPercent = {};
};
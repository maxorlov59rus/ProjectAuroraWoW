#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class LootSystemManager {
public:
    LootSystemManager();
    ~LootSystemManager();
    static LootSystemManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    float DropRate() const { return m_dropRate; }
    // Runtime override from .loot droprate; rejected above MaxDropRate().
    bool SetDropRate(float rate);
    float MaxDropRate() const { return m_maxDropRate; }

private:
    bool m_enabled = true;
    float m_dropRate = {};
    float m_maxDropRate = {};
};


extern "C" {
    __declspec(dllexport) void __stdcall InitializeLootSystemModule();
    __declspec(dllexport) void __stdcall FinalizeLootSystemModule();
}

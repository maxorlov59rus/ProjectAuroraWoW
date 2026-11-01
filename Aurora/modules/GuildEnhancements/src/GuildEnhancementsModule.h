#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler; class Guild;

class GuildEnhancementsManager {
public:
    GuildEnhancementsManager();
    ~GuildEnhancementsManager();
    static GuildEnhancementsManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
};

extern "C" {
    __declspec(dllexport) void __stdcall InitializeGuildEnhancementsModule();
    __declspec(dllexport) void __stdcall FinalizeGuildEnhancementsModule();
}

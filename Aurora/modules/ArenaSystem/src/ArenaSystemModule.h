#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class ArenaSystemManager {
public:
    static ArenaSystemManager& Instance() { static ArenaSystemManager inst; return inst; }
    bool Initialize() { LOG_INFO("ArenaSystem", "Arena system initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("ArenaSystem", "Player %s logged in - arena system available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("ArenaSystem", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterArenaSystemCommands();
bool HandleArenaQueueCommand(ChatHandler* h, const std::string& a);
bool HandleArenaRankCommand(ChatHandler* h, const std::string& a);
bool HandleArenaStatsCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeArenaSystemModule();
    __declspec(dllexport) void __stdcall FinalizeArenaSystemModule();
}
#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler; struct Guild;

class GuildEnhancementsManager {
public:
    static GuildEnhancementsManager& Instance() { static GuildEnhancementsManager inst; return inst; }
    bool Initialize() { LOG_INFO("GuildEnhancements", "Guild enhancements initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("GuildEnhancements", "Player %s logged in - guild enhancements available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("GuildEnhancements", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterGuildEnhancementsCommands();
bool HandleGuildBankCommand(ChatHandler* h, const std::string& a);
bool HandleGuildRankCommand(ChatHandler* h, const std::string& a);
bool HandleGuildInfoCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeGuildEnhancementsModule();
    __declspec(dllexport) void __stdcall FinalizeGuildEnhancementsModule();
}
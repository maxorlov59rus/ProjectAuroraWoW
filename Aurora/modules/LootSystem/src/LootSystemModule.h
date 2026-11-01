#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class LootSystemManager {
public:
    static LootSystemManager& Instance() { static LootSystemManager inst; return inst; }
    bool Initialize() { LOG_INFO("LootSystem", "Loot system initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("LootSystem", "Player %s logged in - loot system available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("LootSystem", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterLootSystemCommands();
bool HandleSetDropRateCommand(ChatHandler* h, const std::string& a);
bool HandleLootInfoCommand(ChatHandler* h, const std::string& a);
bool HandleResetLootCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeLootSystemModule();
    __declspec(dllexport) void __stdcall FinalizeLootSystemModule();
}
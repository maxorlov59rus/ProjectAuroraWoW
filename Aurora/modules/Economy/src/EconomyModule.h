#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class EconomyManager {
public:
    static EconomyManager& Instance() { static EconomyManager inst; return inst; }
    bool Initialize() { LOG_INFO("Economy", "Economy system initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("Economy", "Player %s logged in - economy system available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("Economy", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterEconomyCommands();
bool HandleAuctionCommand(ChatHandler* h, const std::string& a);
bool HandleCurrencyCommand(ChatHandler* h, const std::string& a);
bool HandleExchangeCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeEconomyModule();
    __declspec(dllexport) void __stdcall FinalizeEconomyModule();
}
#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class AdminToolsManager {
public:
    static AdminToolsManager& Instance() { static AdminToolsManager inst; return inst; }
    bool Initialize() { LOG_INFO("AdminTools", "Admin tools initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("AdminTools", "Player %s logged in - admin tools available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("AdminTools", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterAdminToolsCommands();
bool HandleGmCommand(ChatHandler* h, const std::string& a);
bool HandleAdminPanelCommand(ChatHandler* h, const std::string& a);
bool HandleServerInfoCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeAdminToolsModule();
    __declspec(dllexport) void __stdcall FinalizeAdminToolsModule();
}
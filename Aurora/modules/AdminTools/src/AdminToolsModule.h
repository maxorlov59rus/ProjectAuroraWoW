#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class AdminToolsManager {
public:
    AdminToolsManager();
    ~AdminToolsManager();
    static AdminToolsManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    bool RequireSession() const { return m_requireSession; }

private:
    bool m_enabled = true;
    bool m_requireSession = {};
};


extern "C" {
    __declspec(dllexport) void __stdcall InitializeAdminToolsModule();
    __declspec(dllexport) void __stdcall FinalizeAdminToolsModule();
}

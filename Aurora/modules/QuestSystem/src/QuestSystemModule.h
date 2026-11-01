#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class QuestSystemManager {
public:
    QuestSystemManager();
    ~QuestSystemManager();
    static QuestSystemManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;

private:
    bool m_enabled = true;
};


extern "C" {
    __declspec(dllexport) void __stdcall InitializeQuestSystemModule();
    __declspec(dllexport) void __stdcall FinalizeQuestSystemModule();
}

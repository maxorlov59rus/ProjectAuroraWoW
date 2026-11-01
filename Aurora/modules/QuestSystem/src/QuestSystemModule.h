#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class QuestSystemManager {
public:
    static QuestSystemManager& Instance() { static QuestSystemManager inst; return inst; }
    bool Initialize() { LOG_INFO("QuestSystem", "Quest system initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("QuestSystem", "Player %s logged in - quest system available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("QuestSystem", "Player %s leveled up", p->GetName().c_str()); }
};

void RegisterQuestSystemCommands();
bool HandleCreateQuestCommand(ChatHandler* h, const std::string& a);
bool HandleQuestInfoCommand(ChatHandler* h, const std::string& a);
bool HandleCompleteQuestCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeQuestSystemModule();
    __declspec(dllexport) void __stdcall FinalizeQuestSystemModule();
}
#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

struct BotInfo
{
    std::string name;
    std::string className;
    std::string race;
    std::string luaScript;
    bool isRunning = false;
};

class AuroraBotManager
{
public:
    static AuroraBotManager& Instance() { static AuroraBotManager inst; return inst; }
    uint64_t CreateBot(const std::string& name, const std::string& className, const std::string& race, const std::string& luaScript = "");
    bool DeleteBot(uint64_t guid);
    bool GetBot(uint64_t guid, BotInfo& out);
    std::vector<BotInfo> ListBots() const;
    bool SetRunning(uint64_t guid, bool running);
private:
    std::map<uint64_t, BotInfo> m_bots;
    uint64_t m_nextGuid = 1;
    mutable std::mutex m_mutex;
};

void RegisterBotCommands();
bool HandleCreateBotCommand(ChatHandler* h, const std::string& a);
bool HandleDelBotCommand(ChatHandler* h, const std::string& a);
bool HandleListBotsCommand(ChatHandler* h, const std::string& a);
bool HandleBotAiCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeAuroraBotsModule();
    __declspec(dllexport) void __stdcall FinalizeAuroraBotsModule();
}
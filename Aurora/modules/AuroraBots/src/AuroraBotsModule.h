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

    bool Initialize();
    bool IsEnabled() const { return m_enabled; }
    uint32_t MaxBots() const { return m_maxBots; }

    // Creation rules taken from aurora_bots.conf: an entry whose class or race is
    // not on the configured list is refused, so the lists are the actual guard
    // rather than documentation of what the registry "usually" holds.
    bool IsClassAllowed(const std::string& className) const;
    bool IsRaceAllowed(const std::string& race) const;
    // Script handed to a bot created without an explicit one ("" = none).
    std::string const& DefaultScript() const { return m_defaultScript; }
    // Whether a freshly created bot starts its AI right away.
    bool StartRunning() const { return m_startRunning; }

private:
    bool m_enabled = true;
    uint32_t m_maxBots = 20;
    std::vector<std::string> m_allowedClasses;
    std::vector<std::string> m_allowedRaces;
    std::string m_defaultScript;
    bool m_startRunning = true;
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
/*
 * AuroraBotManager — минимальный менеджер ботов.
 * В реальном проекте здесь будут CreatePlayer, DB-хранилище и т.д.
*/
#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>

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
    uint64_t CreateBot(const std::string&, const std::string&, const std::string&, const std::string& = "") { return 0; }
    bool DeleteBot(uint64_t) { return true; }
    bool GetBot(uint64_t, BotInfo&) { return false; }
    std::vector<BotInfo> ListBots() { return {}; }
    bool SetRunning(uint64_t, bool) { return true; }
private:
    std::map<uint64_t, BotInfo> m_bots;
    uint64_t m_nextGuid = 1;
    std::mutex m_mutex;
};

// Минимальные обертки для команд (заглушки, будут переопределены в CMake-сборочном процессе)
void RegisterBotCommands() {}
bool HandleCreateBotCmd(const std::string&) { return true; }
bool HandleDelBotCmd(const std::string&) { return true; }
bool HandleListBotsCmd(const std::string&) { return true; }
bool HandleBotAiCmd(const std::string&) { return true; }

#endif // AURORA_BOT_MANAGER_H
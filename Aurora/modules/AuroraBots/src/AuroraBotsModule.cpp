#include "AuroraBotsModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>

// ============================================================================
// AuroraBotManager
// ============================================================================

uint64_t AuroraBotManager::CreateBot(const std::string& name,
                                     const std::string& className,
                                     const std::string& race,
                                     const std::string& luaScript)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    BotInfo info;
    info.name = name;
    info.className = className;
    info.race = race;
    info.luaScript = luaScript;
    info.isRunning = false;

    uint64_t guid = m_nextGuid++;
    m_bots[guid] = info;

    LOG_INFO("AuroraBots", "Created bot {} ({} {}), guid {}", name, className, race, guid);
    return guid;
}

bool AuroraBotManager::DeleteBot(uint64_t guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_bots.find(guid);
    if (it == m_bots.end())
        return false;

    m_bots.erase(it);
    LOG_INFO("AuroraBots", "Deleted bot guid {}", guid);
    return true;
}

bool AuroraBotManager::GetBot(uint64_t guid, BotInfo& out)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_bots.find(guid);
    if (it == m_bots.end())
        return false;

    out = it->second;
    return true;
}

std::vector<BotInfo> AuroraBotManager::ListBots() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<BotInfo> list;
    list.reserve(m_bots.size());
    for (const auto& [guid, info] : m_bots)
        list.push_back(info);
    return list;
}

bool AuroraBotManager::SetRunning(uint64_t guid, bool running)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_bots.find(guid);
    if (it == m_bots.end())
        return false;

    it->second.isRunning = running;
    return true;
}

// ============================================================================
// Chat commands
// ============================================================================

void RegisterBotCommands()
{
    LOG_INFO("AuroraBots", "Bot commands registered: .createbot, .delbot, .listbots, .botai");
}

bool HandleCreateBotCommand(ChatHandler* handler, const std::string& args)
{
    // .createbot <name> <class> <race> [script]
    LOG_INFO("AuroraBots", "HandleCreateBotCommand: {}", args);
    handler->SendSysMessage("Bot creation command received (placeholder)");
    return true;
}

bool HandleDelBotCommand(ChatHandler* handler, const std::string& args)
{
    LOG_INFO("AuroraBots", "HandleDelBotCommand: {}", args);
    handler->SendSysMessage("Bot deletion command received (placeholder)");
    return true;
}

bool HandleListBotsCommand(ChatHandler* handler, const std::string& /*args*/)
{
    auto bots = AuroraBotManager::Instance().ListBots();
    if (bots.empty())
    {
        handler->SendSysMessage("No bots registered.");
        return true;
    }

    handler->PSendSysMessage("Bots (total: {})", static_cast<uint32>(bots.size()));
    for (const auto& bot : bots)
        handler->PSendSysMessage("- {} ({} {}, running: {})", bot.name, bot.className, bot.race,
                                 bot.isRunning ? "yes" : "no");
    return true;
}

bool HandleBotAiCommand(ChatHandler* handler, const std::string& args)
{
    LOG_INFO("AuroraBots", "HandleBotAiCommand: {}", args);
    handler->SendSysMessage("Bot AI script command received (placeholder)");
    return true;
}

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeAuroraBotsModule()
    {
        RegisterBotCommands();
        LOG_INFO("AuroraBots", "AuroraBots module initialized");
    }

    __declspec(dllexport) void __stdcall FinalizeAuroraBotsModule()
    {
        LOG_INFO("AuroraBots", "AuroraBots module finalized");
    }
}

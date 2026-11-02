#include "AuroraBotsModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>

#include <algorithm>

// ============================================================================
// AuroraBotManager
// ============================================================================

bool AuroraBotManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Bots.Enabled", "AuroraBots");
    m_maxBots = Aurora::Settings::Get<uint32_t>("Aurora.Bots.MaxBots", 20, "AuroraBots");
    m_allowedClasses = Aurora::Settings::List("Aurora.Bots.AllowedClasses",
        "WARRIOR,PALADIN,HUNTER,ROGUE,PRIEST,DEATHKNIGHT,SHAMAN,MAGE,WARLOCK", "AuroraBots");
    m_allowedRaces = Aurora::Settings::List("Aurora.Bots.AllowedRaces",
        "HUMAN,DRAENEI,NIGHT_ELF,GNOME,DWARF,ORC,UNDEAD,TAUREN,TROLL,BLOOD_ELF", "AuroraBots");
    m_defaultScript = Aurora::Settings::Get<std::string>("Aurora.Bots.DefaultScript",
        "scripts/bots/patrol.lua", "AuroraBots");
    m_startRunning = Aurora::Settings::Enabled("Aurora.Bots.StartRunning", "AuroraBots", true);

    if (!m_enabled)
    {
        LOG_INFO("AuroraBots", "AuroraBots disabled by Aurora.Bots.Enabled");
        return true;
    }

    LOG_INFO("AuroraBots",
        "AuroraBots module initialized (max {} bots, {} classes, {} races, default script '{}', start running: {})",
        m_maxBots, m_allowedClasses.size(), m_allowedRaces.size(), m_defaultScript,
        m_startRunning ? "yes" : "no");
    return true;
}

bool AuroraBotManager::IsClassAllowed(const std::string& className) const
{
    return Aurora::Settings::Contains(m_allowedClasses, className);
}

bool AuroraBotManager::IsRaceAllowed(const std::string& race) const
{
    return Aurora::Settings::Contains(m_allowedRaces, race);
}

uint64_t AuroraBotManager::CreateBot(const std::string& name,
                                     const std::string& className,
                                     const std::string& race,
                                     const std::string& luaScript)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_enabled || m_bots.size() >= m_maxBots)
        return 0;

    // The configured lists are the gate: an unlisted class or race would create a
    // bot the module was never meant to run, so it is rejected instead of stored.
    if (!Aurora::Settings::Contains(m_allowedClasses, className))
        return 0;

    if (!Aurora::Settings::Contains(m_allowedRaces, race))
        return 0;

    BotInfo info;
    info.name = name;
    info.className = className;
    info.race = race;
    info.luaScript = luaScript.empty() ? m_defaultScript : luaScript;
    info.isRunning = m_startRunning;

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

namespace
{
    class AuroraBotsWorldScript : public WorldScript
    {
    public:
        AuroraBotsWorldScript() : WorldScript("AuroraBotsWorldScript") { }

        void OnStartup() override
        {
            AuroraBotManager::Instance().Initialize();
        }
    };


}

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

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddAuroraBotsScripts()
{
    new AuroraBotsWorldScript();
}

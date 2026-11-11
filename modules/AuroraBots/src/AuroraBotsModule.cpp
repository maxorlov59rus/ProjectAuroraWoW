/*
 * AuroraBotsModule - loadable AzerothCore module
 *
 * Registers chat commands (.createbot, .delbot, .listbots, .botai) and keeps
 * the in-memory bot registry. Bots are not yet materialised in the world:
 * CreateBot only records the request (see TODO in CreateBot).
 */
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace {

struct BotInfo
{
    std::string name;
    std::string className;
    std::string race;
    std::string luaScript;
    bool isRunning = false;
}

class BotRegistry
{
public:
    static BotRegistry& Instance()
    {
        static BotRegistry instance;
        return instance;
    }

    bool Initialize()
    {
        m_enabled = Aurora::Settings::Enabled("Aurora.Bots.Enabled", "AuroraBots");
        m_maxBots = Aurora::Settings::Bounded<uint32>("Aurora.Bots.MaxBots", 20, 0u, 500u, "AuroraBots");
        m_allowedClasses = Aurora::Settings::List("Aurora.Bots.AllowedClasses",
            "WARRIOR,PALADIN,HUNTER,ROGUE,PRIEST,DEATHKNIGHT,SHAMAN,MAGE,WARLOCK", "AuroraBots");
        m_allowedRaces = Aurora::Settings::List("Aurora.Bots.AllowedRaces",
            "HUMAN,DRAENEI,NIGHT_ELF,GNOME,DWARF,ORC,UNDEAD,TAUREN,TROLL,BLOOD_ELF", "AuroraBots");
        m_defaultScript = Aurora::Settings::Get<std::string>("Aurora.Bots.DefaultScript",
            "patrol", "AuroraBots");
        m_startRunning = Aurora::Settings::Enabled("Aurora.Bots.StartRunning", "AuroraBots", true);

        if (!m_enabled)
        {
            Aurora::Logging::Info("AuroraBots", "disabled by Aurora.Bots.Enabled");
            return true;
        }

        Aurora::Logging::Info("AuroraBots", Acore::StringFormat(
            "module loaded (max {} bots, {} classes, {} races, default script '{}', start running: {})",
            m_maxBots, m_allowedClasses.size(), m_allowedRaces.size(), m_defaultScript,
            m_startRunning ? "yes" : "no"));
        return true;
    }

    uint64_t CreateBot(std::string const& name, std::string const& className,
                       std::string const& race, std::string const& luaScript)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_enabled || m_bots.size() >= m_maxBots)
            return 0;

        // The configured lists are the gate: an unlisted class or race would
        // create a bot the module was never meant to run.
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

        Aurora::Logging::Info("AuroraBots", Acore::StringFormat(
            "created bot '{}' ({} {}), guid {}", name, className, race, guid));
        return guid;
    }

    bool DeleteBot(uint64_t guid)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_bots.erase(guid) > 0;
    }

    bool GetBot(uint64_t guid, BotInfo& out)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_bots.find(guid);
        if (it == m_bots.end())
            return false;
        out = it->second;
        return true;
    }

    std::vector<BotInfo> ListBots() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<BotInfo> list;
        list.reserve(m_bots.size());
        for (auto const& [guid, info] : m_bots)
            list.push_back(info);
        return list;
    }

    bool SetRunning(uint64_t guid, bool running)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_bots.find(guid);
        if (it == m_bots.end())
            return false;
        it->second.isRunning = running;
        return true;
    }

private:
    std::map<uint64_t, BotInfo> m_bots;
    uint64_t m_nextGuid = 1;
    std::mutex m_mutex;

    bool m_enabled = true;
    uint32 m_maxBots = 20;
    std::vector<std::string> m_allowedClasses;
    std::vector<std::string> m_allowedRaces;
    std::string m_defaultScript = "patrol";
    bool m_startRunning = true;
}

// .createbot <name> <class> <race> [script]
bool HandleCreateBotCommand(ChatHandler* handler, std::string const& args)
{
    std::istringstream stream(args);
    std::string name, className, race, script;
    if (!(stream >> name >> className >> race))
    {
        handler->SendSysMessage("Usage: .createbot <name> <class> <race> [script]");
        return false;
    }
    stream >> script;

    // TODO: spawn the real player object once map creation is wired up; for
    // now the registry records the intent and returns the internal guid.
    uint64_t guid = BotRegistry::Instance().CreateBot(name, className, race, script);
    if (!guid)
    {
        handler->SendSysMessage("Bot creation rejected (disabled, limit reached, or class/race not allowed).");
        return false;
    }

    handler->PSendSysMessage("Bot '{}' registered with guid {}.", name, guid);
    return true;
}

// .delbot <guid>
bool HandleDelBotCommand(ChatHandler* handler, std::string const& args)
{
    uint64_t guid = 0;
    std::istringstream stream(args);
    if (!(stream >> guid) || !guid)
    {
        handler->SendSysMessage("Usage: .delbot <guid>");
        return false;
    }

    if (!BotRegistry::Instance().DeleteBot(guid))
    {
        handler->SendSysMessage("No such bot.");
        return false;
    }

    handler->PSendSysMessage("Bot guid {} deleted.", guid);
    return true;
}

// .listbots
bool HandleListBotsCommand(ChatHandler* handler, std::string const& /*args*/)
{
    auto bots = BotRegistry::Instance().ListBots();
    if (bots.empty())
    {
        handler->SendSysMessage("No bots registered.");
        return true;
    }

    handler->PSendSysMessage("Bots (total: {})", static_cast<uint32>(bots.size()));
    for (auto const& bot : bots)
        handler->PSendSysMessage("- {} ({} {}, running: {})", bot.name, bot.className, bot.race,
                                 bot.isRunning ? "yes" : "no");
    return true;
}

// .botai <guid> <script>
bool HandleBotAiCommand(ChatHandler* handler, std::string const& args)
{
    handler->SendSysMessage("Bot AI scripts are not implemented yet.");
    return true;
}

class AuroraBotsWorldScript : public WorldScript
{
public:
    AuroraBotsWorldScript() : WorldScript("AuroraBotsWorldScript") { }

    void OnStartup() override
    {
        BotRegistry::Instance().Initialize();
    }
}

} // namespace

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

/*
 * ClasslessSystem module — implementation
 *
 * Classless characters: the role comes from spent skill points instead of a
 * fixed AzerothCore class. The manager keeps the classless registry in memory
 * and exposes it through chat commands.
 *
 * This module registers one AzerothCore script:
 *   - ClasslessCommandScript : CommandScript (chat commands)
 */

#include "ClasslessSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// ClasslessManager — implementation
// ============================================================================

ClasslessManager::ClasslessManager() = default;

ClasslessManager::~ClasslessManager() = default;

bool ClasslessManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Classless.Enabled", "ClasslessSystem");
    m_pointsPerLevel = Aurora::Settings::Get<uint32_t>("Aurora.Classless.PointsPerLevel", 2, "ClasslessSystem");

    if (!m_enabled)
    {
        LOG_INFO("ClasslessSystem", "ClasslessSystem disabled by Aurora.Classless.Enabled");
        return true;
    }

    LOG_INFO("ClasslessSystem", "ClasslessSystem module initialized ({} skill points per level)",
        m_pointsPerLevel);
    return true;
}

bool ClasslessManager::CreateClasslessPlayer(const std::string& name,
                                             uint32_t level,
                                             uint64_t playerGuid,
                                             const std::string& luaScript)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_enabled || name.empty())
        return false;

    if (playerGuid == 0)
        playerGuid = m_nextGuid++;

    if (m_players.count(playerGuid))
        return false;

    ClasslessInfo info;
    info.name = name;
    info.level = level;
    info.skillPoints = 0;
    info.luaScript = luaScript;

    m_players[playerGuid] = info;

    LOG_INFO("ClasslessSystem", "Registered classless character {} (GUID: {}, level {})",
        name, playerGuid, level);
    return true;
}

bool ClasslessManager::DeleteClasslessPlayer(uint64_t guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end())
        return false;

    m_players.erase(it);
    LOG_INFO("ClasslessSystem", "Deleted classless character GUID: {}", guid);
    return true;
}

bool ClasslessManager::GetClasslessInfo(uint64_t guid, ClasslessInfo& out)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end())
        return false;

    out = it->second;
    return true;
}

std::vector<ClasslessInfo> ClasslessManager::ListClassless() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ClasslessInfo> list;
    list.reserve(m_players.size());
    for (auto const& [guid, info] : m_players)
        list.push_back(info);
    return list;
}

bool ClasslessManager::SetSkillPoints(uint64_t guid, uint32_t points)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end())
        return false;

    it->second.skillPoints = points;
    return true;
}

bool ClasslessManager::AddSkillPoints(uint64_t guid, uint32_t points)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end())
        return false;

    it->second.skillPoints += points;
    return true;
}

// ============================================================================
// Helpers
// ============================================================================

uint32_t GetClassMaskForName(const std::string& name)
{
    // Class masks are the per-class bits of the CLASS_* ids defined in
    // SharedDefines.h (the ids are 1-based).
    auto mask = [](Classes c) { return c == CLASS_NONE ? 0u : (1u << (c - 1)); };

    if (name == "WARRIOR")     return mask(CLASS_WARRIOR);
    if (name == "PALADIN")     return mask(CLASS_PALADIN);
    if (name == "HUNTER")      return mask(CLASS_HUNTER);
    if (name == "ROGUE")       return mask(CLASS_ROGUE);
    if (name == "PRIEST")      return mask(CLASS_PRIEST);
    if (name == "DEATHKNIGHT") return mask(CLASS_DEATH_KNIGHT);
    if (name == "SHAMAN")      return mask(CLASS_SHAMAN);
    if (name == "MAGE")        return mask(CLASS_MAGE);
    if (name == "WARLOCK")     return mask(CLASS_WARLOCK);
    return 0;
}

bool IsClassless(Player* pPlayer)
{
    if (!pPlayer)
        return false;

    auto list = ClasslessManager::Instance().ListClassless();
    return std::find_if(list.begin(), list.end(),
        [pPlayer](ClasslessInfo const& info) { return info.name == pPlayer->GetName(); }) != list.end();
}

// ============================================================================
// CommandScript — real command registration
// ============================================================================

class ClasslessCommandScript : public CommandScript
{
public:
    ClasslessCommandScript() : CommandScript("ClasslessCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable classlessTable =
        {
            { "create", HandleCreateClasslessCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "delete", HandleDeleteClasslessCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "skills", HandleSetSkillsCommand,       rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "list",   HandleListClasslessCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info",   HandleClasslessInfoCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "classless", classlessTable }
        };

        return commandTable;
    }

private:
    // ---------- .classless create <name> <level> ----------
    static bool HandleCreateClasslessCommand(ChatHandler* handler, std::string name, uint32 level)
    {
        if (ClasslessManager::Instance().CreateClasslessPlayer(name, level))
            handler->PSendSysMessage("Classless character '{}' (level {}) registered.", name, level);
        else
            handler->PSendSysMessage("Cannot register classless character '{}'.", name);

        return true;
    }

    // ---------- .classless delete <guid> ----------
    static bool HandleDeleteClasslessCommand(ChatHandler* handler, uint64 guid)
    {
        if (ClasslessManager::Instance().DeleteClasslessPlayer(guid))
            handler->PSendSysMessage("Classless character {} deleted.", guid);
        else
            handler->PSendSysMessage("Classless character {} not found.", guid);

        return true;
    }

    // ---------- .classless skills <guid> <points> ----------
    static bool HandleSetSkillsCommand(ChatHandler* handler, uint64 guid, uint32 points)
    {
        if (ClasslessManager::Instance().SetSkillPoints(guid, points))
            handler->PSendSysMessage("Skill points of {} set to {}.", guid, points);
        else
            handler->PSendSysMessage("Classless character {} not found.", guid);

        return true;
    }

    // ---------- .classless list ----------
    static bool HandleListClasslessCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto list = ClasslessManager::Instance().ListClassless();
        if (list.empty())
        {
            handler->SendSysMessage("No classless characters registered.");
            return true;
        }

        handler->PSendSysMessage("Classless characters (total: {}):", static_cast<uint32>(list.size()));
        for (auto const& info : list)
            handler->PSendSysMessage("- {} (level {}, skill points: {})",
                info.name, info.level, info.skillPoints);

        return true;
    }

    // ---------- .classless info <guid> ----------
    static bool HandleClasslessInfoCommand(ChatHandler* handler, uint64 guid)
    {
        ClasslessInfo info{};
        if (ClasslessManager::Instance().GetClasslessInfo(guid, info))
            handler->PSendSysMessage("{}: level {}, skill points {}, script '{}'",
                info.name, info.level, info.skillPoints, info.luaScript);
        else
            handler->PSendSysMessage("Classless character {} not found.", guid);

        return true;
    }
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeClasslessSystemModule()
    {
        LOG_INFO("ClasslessSystem", "ClasslessSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeClasslessSystemModule()
    {
        LOG_INFO("ClasslessSystem", "ClasslessSystem module finalized");
    }
}

// ============================================================================
// Static instance — auto-registers via constructor
// ============================================================================

namespace
{
    class ClasslessWorldScript : public WorldScript
    {
    public:
        ClasslessWorldScript() : WorldScript("ClasslessWorldScript") { }

        void OnStartup() override
        {
            ClasslessManager::Instance().Initialize();
        }
    };



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

void AddClasslessSystemScripts()
{
    new ClasslessCommandScript();
    new ClasslessWorldScript();
}

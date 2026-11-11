/*
 * ClasslessSystem module - implementation
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

#include <algorithm>

// ============================================================================
// ClasslessManager - implementation
// ============================================================================

ClasslessManager::ClasslessManager() = default;

ClasslessManager::~ClasslessManager() = default;

bool ClasslessManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Classless.Enabled", "ClasslessSystem");
    m_pointsPerLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.Classless.PointsPerLevel", 2, 0, 100, "ClasslessSystem");
    m_maxLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.Classless.MaxLevel", 80, 1, 80, "ClasslessSystem");
    // 0 is a meaningful value here: it means "no cap", so it must not be clamped away.
    m_maxSkillPoints = Aurora::Settings::Get<uint32_t>("Aurora.Classless.MaxSkillPoints", 0, "ClasslessSystem");
    m_defaultScript = Aurora::Settings::Get<std::string>("Aurora.Classless.DefaultScript", "", "ClasslessSystem");

    if (!m_enabled)
    {
        LOG_INFO("ClasslessSystem", "ClasslessSystem disabled by Aurora.Classless.Enabled");
        return true;
    }

    LOG_INFO("ClasslessSystem",
        "ClasslessSystem module initialized ({} skill points per level, max level {}, skill point cap {}, default script '{}')",
        m_pointsPerLevel, m_maxLevel, m_maxSkillPoints, m_defaultScript);
    return true;
}

bool ClasslessManager::CreateClasslessPlayer(const std::string& name,
                                             uint32_t level,
                                             uint64_t playerGuid,
                                             const std::string& luaScript,
                                             uint64_t* outGuid)
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
    // A classless character above the configured ceiling would earn points the
    // module is not meant to hand out, so the level is capped instead of refused.
    info.level = m_maxLevel ? std::min(level, m_maxLevel) : level;
    // A classless character starts with the points the configured rate grants for
    // its level; leaving them at 0 made PointsPerLevel a documentation-only option.
    info.skillPoints = info.level * m_pointsPerLevel;
    if (m_maxSkillPoints)
        info.skillPoints = std::min(info.skillPoints, m_maxSkillPoints);
    info.luaScript = luaScript.empty() ? m_defaultScript : luaScript;

    m_players[playerGuid] = info;
    if (outGuid)
        *outGuid = playerGuid;

    LOG_INFO("ClasslessSystem", "Registered classless character {} (GUID: {}, level {}, {} skill points)",
        name, playerGuid, info.level, info.skillPoints);
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

    // MaxSkillPoints is the ceiling for the whole registry, so a GM command
    // cannot push a character past it either.
    if (m_maxSkillPoints && points > m_maxSkillPoints)
        points = m_maxSkillPoints;

    it->second.skillPoints = points;
    return true;
}

bool ClasslessManager::AddSkillPoints(uint64_t guid, uint32_t points)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_players.find(guid);
    if (it == m_players.end())
        return false;

    uint32_t total = it->second.skillPoints + points;
    if (m_maxSkillPoints)
        total = std::min(total, m_maxSkillPoints);

    it->second.skillPoints = total;
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
// CommandScript - real command registration
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
        ClasslessManager& mgr = ClasslessManager::Instance();
        uint64_t guid = 0;
        if (!mgr.CreateClasslessPlayer(name, level, 0, "", &guid))
        {
            handler->PSendSysMessage("Cannot register classless character '{}'.", name);
            return true;
        }

        // Report what was actually stored: the level may have been capped by
        // MaxLevel and the points come from PointsPerLevel.
        ClasslessInfo info{};
        mgr.GetClasslessInfo(guid, info);

        handler->PSendSysMessage("Classless character '{}' (GUID: {}, level {}, {} skill points) registered.",
            info.name, guid, info.level, info.skillPoints);
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
// Static instance - auto-registers via constructor
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

void AddClasslessSystemScripts()
{
    new ClasslessCommandScript();
    new ClasslessWorldScript();
}

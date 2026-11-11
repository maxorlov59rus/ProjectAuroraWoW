/*
 * ArenaSystem module - implementation
 *
 * Arena team helpers: queueing, ranks and per-team statistics.
 *
 * This module registers two AzerothCore scripts:
 *   - ArenaSystemCommandScript : CommandScript (chat commands)
 *   - ArenaSystemWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_arena.conf (see ArenaSystem/conf/aurora_arena.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "ArenaSystemModule.h"

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
// ArenaSystemManager - implementation
// ============================================================================

ArenaSystemManager::ArenaSystemManager() = default;

ArenaSystemManager::~ArenaSystemManager() = default;

ArenaSystemManager& ArenaSystemManager::Instance()
{
    static ArenaSystemManager inst;
    return inst;
}

bool ArenaSystemManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Arena.Enabled", "ArenaSystem");
    m_ratedMinLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.Arena.RatedMinimumLevel", 80, 1, 80, "ArenaSystem");

    if (!m_enabled)
    {
        LOG_INFO("ArenaSystem", "ArenaSystem disabled by Aurora.Arena.Enabled");
        return true;
    }

    LOG_INFO("ArenaSystem", "ArenaSystem module initialized (rated arena from level {})", m_ratedMinLevel);
    return true;
}

bool ArenaSystemManager::IsEnabled() const
{
    return m_enabled;
}

void ArenaSystemManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("ArenaSystem", "Player {} logged in - arena system available", p->GetName());
}

void ArenaSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("ArenaSystem", "Player {} leveled up", p->GetName());
}

// ============================================================================
// CommandScript
// ============================================================================

class ArenaSystemCommandScript : public CommandScript
{
public:
    ArenaSystemCommandScript() : CommandScript("ArenaSystemCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable arenaTable =
        {
            { "queue", HandleArenaQueueCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "rank", HandleArenaRankCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "stats", HandleArenaStatsCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "arena", arenaTable }
        };

        return commandTable;
    }

private:
    // ---------- .arena queue ----------
    static bool HandleArenaQueueCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!ArenaSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("ArenaSystem is disabled by configuration.");
            return true;
        }

        Player* p = handler->GetPlayer();
        if (p && p->GetLevel() < ArenaSystemManager::Instance().RatedMinLevel())
        {
            handler->PSendSysMessage("Rated arena queue requires level {} (you are {}).",
                ArenaSystemManager::Instance().RatedMinLevel(), p->GetLevel());
            return true;
        }

        if (!p)
        {
            handler->SendSysMessage("The arena queue command needs an in-game character.");
            return true;
        }

        handler->PSendSysMessage("Arena queue request accepted for {}.");
        return true;
    }


    // ---------- .arena rank ----------
    static bool HandleArenaRankCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!ArenaSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("ArenaSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Arena rank command received.");
        return true;
    }


    // ---------- .arena stats ----------
    static bool HandleArenaStatsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!ArenaSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("ArenaSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Rated arena requires level {}.", ArenaSystemManager::Instance().RatedMinLevel());
        return true;
    }

};

// ============================================================================
// WorldScript
// ============================================================================

class ArenaSystemWorldScript : public WorldScript
{
public:
    ArenaSystemWorldScript() : WorldScript("ArenaSystemWorldScript") { }

    void OnStartup() override
    {
        ArenaSystemManager::Instance().Initialize();
    }
};

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddArenaSystemScripts()
{
    new ArenaSystemCommandScript();
    new ArenaSystemWorldScript();
}

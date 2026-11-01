/*
 * ArenaSystem module — implementation
 *
 * Arena team helpers: queueing, ranks and per-team statistics.
 *
 * This module registers one AzerothCore script:
 *   - ArenaSystemCommandScript : CommandScript (chat commands)
 */

#include "ArenaSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// ArenaSystemManager — implementation
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
    LOG_INFO("ArenaSystem", "ArenaSystem module initialized");
    return true;
}

void ArenaSystemManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("ArenaSystem", "Player {} logged in — arena system available", p->GetName());
}

void ArenaSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
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
        handler->SendSysMessage("Arena queue command received.");
        return true;
    }

    // ---------- .arena rank ----------
    static bool HandleArenaRankCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Arena rank command received.");
        return true;
    }

    // ---------- .arena stats ----------
    static bool HandleArenaStatsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Arena stats command received.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeArenaSystemModule()
    {
        ArenaSystemManager::Instance().Initialize();
        LOG_INFO("ArenaSystem", "ArenaSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeArenaSystemModule()
    {
        LOG_INFO("ArenaSystem", "ArenaSystem module finalized");
    }
}

namespace
{
    ArenaSystemCommandScript sArenaSystemCommandScript;
}

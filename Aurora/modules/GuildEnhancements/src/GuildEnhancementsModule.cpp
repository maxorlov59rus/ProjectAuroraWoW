/*
 * GuildEnhancements module — implementation
 *
 * Guild improvements: bank, ranks and extended info.
 *
 * This module registers one AzerothCore script:
 *   - GuildEnhancementsCommandScript : CommandScript (chat commands)
 */

#include "GuildEnhancementsModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// GuildEnhancementsManager — implementation
// ============================================================================

GuildEnhancementsManager::GuildEnhancementsManager() = default;

GuildEnhancementsManager::~GuildEnhancementsManager() = default;

GuildEnhancementsManager& GuildEnhancementsManager::Instance()
{
    static GuildEnhancementsManager inst;
    return inst;
}

bool GuildEnhancementsManager::Initialize()
{
    LOG_INFO("GuildEnhancements", "GuildEnhancements module initialized");
    return true;
}

void GuildEnhancementsManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("GuildEnhancements", "Player {} logged in — guild enhancements available", p->GetName());
}

void GuildEnhancementsManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;
    LOG_INFO("GuildEnhancements", "Player {} leveled up", p->GetName());
}


// ============================================================================
// CommandScript
// ============================================================================

class GuildEnhancementsCommandScript : public CommandScript
{
public:
    GuildEnhancementsCommandScript() : CommandScript("GuildEnhancementsCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable guildxTable =
        {
            { "bank", HandleGuildBankCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "rank", HandleGuildRankCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info", HandleGuildInfoCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "guildx", guildxTable }
        };

        return commandTable;
    }

private:
    // ---------- .guildx bank ----------
    static bool HandleGuildBankCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Guild bank command received.");
        return true;
    }

    // ---------- .guildx rank ----------
    static bool HandleGuildRankCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Guild rank command received.");
        return true;
    }

    // ---------- .guildx info ----------
    static bool HandleGuildInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Guild info command received.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeGuildEnhancementsModule()
    {
        GuildEnhancementsManager::Instance().Initialize();
        LOG_INFO("GuildEnhancements", "GuildEnhancements module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeGuildEnhancementsModule()
    {
        LOG_INFO("GuildEnhancements", "GuildEnhancements module finalized");
    }
}

namespace
{
    GuildEnhancementsCommandScript sGuildEnhancementsCommandScript;
}

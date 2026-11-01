/*
 * GuildEnhancements module - implementation
 *
 * Guild improvements: bank, ranks and extended info.
 *
 * This module registers two AzerothCore scripts:
 *   - GuildEnhancementsCommandScript : CommandScript (chat commands)
 *   - GuildEnhancementsWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_guild.conf (see GuildEnhancements/conf/aurora_guild.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "GuildEnhancementsModule.h"

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
// GuildEnhancementsManager - implementation
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
    m_enabled = Aurora::Settings::Enabled("Aurora.GuildEnhancements.Enabled", "GuildEnhancements");
    m_bankTabCost = Aurora::Settings::Get<uint32_t>("Aurora.GuildEnhancements.BankTabCost", 100000, "GuildEnhancements");

    if (!m_enabled)
    {
        LOG_INFO("GuildEnhancements", "GuildEnhancements disabled by Aurora.GuildEnhancements.Enabled");
        return true;
    }

    LOG_INFO("GuildEnhancements", "GuildEnhancements module initialized (bank tab {} copper)", m_bankTabCost);
    return true;
}

bool GuildEnhancementsManager::IsEnabled() const
{
    return m_enabled;
}

void GuildEnhancementsManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("GuildEnhancements", "Player {} logged in - guild enhancements available", p->GetName());
}

void GuildEnhancementsManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
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
        if (!GuildEnhancementsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("GuildEnhancements is disabled by configuration.");
            return true;
        }

        uint32 const cost = GuildEnhancementsManager::Instance().BankTabCost();

        handler->PSendSysMessage("A new guild bank tab costs {} gold, {} silver, {} copper.", cost / 10000, (cost / 100) % 100, cost % 100);
        return true;
    }


    // ---------- .guildx rank ----------
    static bool HandleGuildRankCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!GuildEnhancementsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("GuildEnhancements is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Guild rank command received.");
        return true;
    }


    // ---------- .guildx info ----------
    static bool HandleGuildInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!GuildEnhancementsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("GuildEnhancements is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Guild info command received.");
        return true;
    }

};

// ============================================================================
// WorldScript
// ============================================================================

class GuildEnhancementsWorldScript : public WorldScript
{
public:
    GuildEnhancementsWorldScript() : WorldScript("GuildEnhancementsWorldScript") { }

    void OnStartup() override
    {
        GuildEnhancementsManager::Instance().Initialize();
    }
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeGuildEnhancementsModule()
    {
        LOG_INFO("GuildEnhancements", "GuildEnhancements module loaded (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeGuildEnhancementsModule()
    {
        LOG_INFO("GuildEnhancements", "GuildEnhancements module finalized");
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

void AddGuildEnhancementsScripts()
{
    new GuildEnhancementsCommandScript();
    new GuildEnhancementsWorldScript();
}

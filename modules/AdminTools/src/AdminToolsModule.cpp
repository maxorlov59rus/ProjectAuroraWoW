/*
 * AdminTools module - implementation
 *
 * GM helpers: admin panel, server info and an umbrella command.
 *
 * This module registers two AzerothCore scripts:
 *   - AdminToolsCommandScript : CommandScript (chat commands)
 *   - AdminToolsWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_admintools.conf (see AdminTools/conf/aurora_admintools.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "AdminToolsModule.h"

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
// AdminToolsManager - implementation
// ============================================================================

AdminToolsManager::AdminToolsManager() = default;

AdminToolsManager::~AdminToolsManager() = default;

AdminToolsManager& AdminToolsManager::Instance()
{
    static AdminToolsManager inst;
    return inst;
}

bool AdminToolsManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.AdminTools.Enabled", "AdminTools");
    m_requireSession = Aurora::Settings::Enabled("Aurora.AdminTools.RequireSession", "AdminTools", true);

    if (!m_enabled)
    {
        LOG_INFO("AdminTools", "AdminTools disabled by Aurora.AdminTools.Enabled");
        return true;
    }

    LOG_INFO("AdminTools", "AdminTools module initialized (console access: {})",
        m_requireSession ? "denied" : "allowed");
    return true;
}

bool AdminToolsManager::IsEnabled() const
{
    return m_enabled;
}

void AdminToolsManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("AdminTools", "Player {} logged in - admin tools available", p->GetName());
}

void AdminToolsManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("AdminTools", "Player {} leveled up", p->GetName());
}

// ============================================================================
// CommandScript
// ============================================================================

class AdminToolsCommandScript : public CommandScript
{
public:
    AdminToolsCommandScript() : CommandScript("AdminToolsCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable adminTable =
        {
            { "adminpanel", HandleAdminPanelCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "serverinfo", HandleServerInfoCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "help", HandleAdminCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "admin", adminTable }
        };

        return commandTable;
    }

private:
    // ---------- .admin adminpanel ----------
    static bool HandleAdminPanelCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!AdminToolsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("AdminTools is disabled by configuration.");
            return true;
        }

        if (handler->IsConsole() && AdminToolsManager::Instance().RequireSession())
        {
            handler->SendSysMessage("The admin panel needs an in-game GM session.");
            return true;
        }

        handler->PSendSysMessage("Admin panel is {}.", AdminToolsManager::Instance().RequireSession() ? "restricted to in-game GMs" : "also available from the console");
        return true;
    }


    // ---------- .admin serverinfo ----------
    static bool HandleServerInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!AdminToolsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("AdminTools is disabled by configuration.");
            return true;
        }

        if (handler->IsConsole() && AdminToolsManager::Instance().RequireSession())
        {
            handler->SendSysMessage("The admin commands need an in-game GM session.");
            return true;
        }

        handler->SendSysMessage("Server info command received.");
        return true;
    }


    // ---------- .admin help ----------
    static bool HandleAdminCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!AdminToolsManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("AdminTools is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Commands: .admin adminpanel, .admin serverinfo, .admin help.");
        return true;
    }

}

// ============================================================================
// WorldScript
// ============================================================================

class AdminToolsWorldScript : public WorldScript
{
public:
    AdminToolsWorldScript() : WorldScript("AdminToolsWorldScript") { }

    void OnStartup() override
    {
        AdminToolsManager::Instance().Initialize();
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

void AddAdminToolsScripts()
{
    new AdminToolsCommandScript();
    new AdminToolsWorldScript();
}

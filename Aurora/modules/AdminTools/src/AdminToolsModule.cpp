/*
 * AdminTools module — implementation
 *
 * GM commands: .admin, .adminpanel, .serverinfo.
 *
 * This module registers one AzerothCore script:
 *   - AdminToolsCommandScript : CommandScript (chat commands)
 */

#include "AdminToolsModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

#include <chrono>

// ============================================================================
// AdminToolsManager — implementation
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
    LOG_INFO("AdminTools", "AdminTools module initialized");
    return true;
}

void AdminToolsManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("AdminTools", "Player {} logged in - admin tools available", p->GetName());
}

void AdminToolsManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
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
            { "adminpanel", HandleAdminPanelCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "serverinfo", HandleServerInfoCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "admin",      HandleAdminCommand,        rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "admin", adminTable }
        };

        return commandTable;
    }

private:
    static bool HandleAdminPanelCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->PSendSysMessage("Admin panel command received.");
        return true;
    }

    static bool HandleServerInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->PSendSysMessage("Server uptime: {} minutes.",
            std::chrono::duration_cast<std::chrono::minutes>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        return true;
    }

    static bool HandleAdminCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Admin panel opened.");
        return true;
    }
};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeAdminToolsModule()
    {
        AdminToolsManager::Instance().Initialize();
        LOG_INFO("AdminTools", "AdminTools module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeAdminToolsModule()
    {
        LOG_INFO("AdminTools", "AdminTools module finalized");
    }
}

namespace
{
    AdminToolsCommandScript sAdminToolsCommandScript;
}

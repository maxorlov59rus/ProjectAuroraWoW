/*
 * QuestSystem module — implementation
 *
 * Custom quest helpers: creation, inspection and completion.
 *
 * This module registers one AzerothCore script:
 *   - QuestSystemCommandScript : CommandScript (chat commands)
 */

#include "QuestSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// QuestSystemManager — implementation
// ============================================================================

QuestSystemManager::QuestSystemManager() = default;

QuestSystemManager::~QuestSystemManager() = default;

QuestSystemManager& QuestSystemManager::Instance()
{
    static QuestSystemManager inst;
    return inst;
}

bool QuestSystemManager::Initialize()
{
    LOG_INFO("QuestSystem", "QuestSystem module initialized");
    return true;
}

void QuestSystemManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("QuestSystem", "Player {} logged in — quest system available", p->GetName());
}

void QuestSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;
    LOG_INFO("QuestSystem", "Player {} leveled up", p->GetName());
}


// ============================================================================
// CommandScript
// ============================================================================

class QuestSystemCommandScript : public CommandScript
{
public:
    QuestSystemCommandScript() : CommandScript("QuestSystemCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable questxTable =
        {
            { "create", HandleCreateQuestCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info", HandleQuestInfoCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "complete", HandleCompleteQuestCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "questx", questxTable }
        };

        return commandTable;
    }

private:
    // ---------- .questx create ----------
    static bool HandleCreateQuestCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Create quest command received.");
        return true;
    }

    // ---------- .questx info ----------
    static bool HandleQuestInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Quest info command received.");
        return true;
    }

    // ---------- .questx complete ----------
    static bool HandleCompleteQuestCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Complete quest command received.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeQuestSystemModule()
    {
        QuestSystemManager::Instance().Initialize();
        LOG_INFO("QuestSystem", "QuestSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeQuestSystemModule()
    {
        LOG_INFO("QuestSystem", "QuestSystem module finalized");
    }
}

namespace
{
    QuestSystemCommandScript sQuestSystemCommandScript;
}

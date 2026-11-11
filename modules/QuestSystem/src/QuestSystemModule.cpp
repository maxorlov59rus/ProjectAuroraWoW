/*
 * QuestSystem module - implementation
 *
 * Custom quest helpers: creation, inspection and completion.
 *
 * This module registers two AzerothCore scripts:
 *   - QuestSystemCommandScript : CommandScript (chat commands)
 *   - QuestSystemWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_quest.conf (see QuestSystem/conf/aurora_quest.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "QuestSystemModule.h"

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
// QuestSystemManager - implementation
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
    m_enabled = Aurora::Settings::Enabled("Aurora.Quests.Enabled", "QuestSystem");

    if (!m_enabled)
    {
        LOG_INFO("QuestSystem", "QuestSystem disabled by Aurora.Quests.Enabled");
        return true;
    }

    LOG_INFO("QuestSystem", "QuestSystem module initialized");
    return true;
}

bool QuestSystemManager::IsEnabled() const
{
    return m_enabled;
}

void QuestSystemManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("QuestSystem", "Player {} logged in - quest system available", p->GetName());
}

void QuestSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
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
        if (!QuestSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("QuestSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Create quest command received.");
        return true;
    }


    // ---------- .questx info ----------
    static bool HandleQuestInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!QuestSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("QuestSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Quest info command received.");
        return true;
    }


    // ---------- .questx complete ----------
    static bool HandleCompleteQuestCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!QuestSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("QuestSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Complete quest command received.");
        return true;
    }

};

// ============================================================================
// WorldScript
// ============================================================================

class QuestSystemWorldScript : public WorldScript
{
public:
    QuestSystemWorldScript() : WorldScript("QuestSystemWorldScript") { }

    void OnStartup() override
    {
        QuestSystemManager::Instance().Initialize();
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

void AddQuestSystemScripts()
{
    new QuestSystemCommandScript();
    new QuestSystemWorldScript();
}

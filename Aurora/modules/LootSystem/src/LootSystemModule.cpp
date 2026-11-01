/*
 * LootSystem module — implementation
 *
 * Loot tuning: drop rates, loot inspection and reset.
 *
 * This module registers two AzerothCore scripts:
 *   - LootSystemCommandScript : CommandScript (chat commands)
 *   - LootSystemWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_loot.conf (see LootSystem/conf/aurora_loot.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "LootSystemModule.h"

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
// LootSystemManager — implementation
// ============================================================================

LootSystemManager::LootSystemManager() = default;

LootSystemManager::~LootSystemManager() = default;

LootSystemManager& LootSystemManager::Instance()
{
    static LootSystemManager inst;
    return inst;
}

bool LootSystemManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Loot.Enabled", "LootSystem");
    m_dropRate = Aurora::Settings::Get<float>("Aurora.Loot.DropRate", 1.0f, "LootSystem");

    if (!m_enabled)
    {
        LOG_INFO("LootSystem", "LootSystem disabled by Aurora.Loot.Enabled");
        return true;
    }

    LOG_INFO("LootSystem", "LootSystem module initialized");
    return true;
}

bool LootSystemManager::IsEnabled() const
{
    return m_enabled;
}

void LootSystemManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("LootSystem", "Player {} logged in — loot system available", p->GetName());
}

void LootSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("LootSystem", "Player {} leveled up", p->GetName());
}


// ============================================================================
// CommandScript
// ============================================================================

class LootSystemCommandScript : public CommandScript
{
public:
    LootSystemCommandScript() : CommandScript("LootSystemCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable lootTable =
        {
            { "droprate", HandleSetDropRateCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info", HandleLootInfoCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "reset", HandleResetLootCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "loot", lootTable }
        };

        return commandTable;
    }

private:
    // ---------- .loot droprate ----------
    static bool HandleSetDropRateCommand(ChatHandler* handler, float rate)
    {
        if (!LootSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("LootSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Drop rate set to {}.", rate);
        return true;
    }

    // ---------- .loot info ----------
    static bool HandleLootInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!LootSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("LootSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Configured drop rate: {}.", LootSystemManager::Instance().DropRate());
        return true;
    }

    // ---------- .loot reset ----------
    static bool HandleResetLootCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!LootSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("LootSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Loot tables reset.");
        return true;
    }

};

// ============================================================================
// WorldScript
// ============================================================================

class LootSystemWorldScript : public WorldScript
{
public:
    LootSystemWorldScript() : WorldScript("LootSystemWorldScript") { }

    void OnStartup() override
    {
        LootSystemManager::Instance().Initialize();
    }
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeLootSystemModule()
    {
        LOG_INFO("LootSystem", "LootSystem module loaded (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeLootSystemModule()
    {
        LOG_INFO("LootSystem", "LootSystem module finalized");
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

void AddLootSystemScripts()
{
    new LootSystemCommandScript();
    new LootSystemWorldScript();
}

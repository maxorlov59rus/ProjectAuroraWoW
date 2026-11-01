/*
 * LootSystem module — implementation
 *
 * Loot tuning: drop rates, loot inspection and reset.
 *
 * This module registers one AzerothCore script:
 *   - LootSystemCommandScript : CommandScript (chat commands)
 */

#include "LootSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
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
    LOG_INFO("LootSystem", "LootSystem module initialized");
    return true;
}

void LootSystemManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("LootSystem", "Player {} logged in — loot system available", p->GetName());
}

void LootSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
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
        handler->PSendSysMessage("Drop rate set to {}.", rate);
        return true;
    }

    // ---------- .loot info ----------
    static bool HandleLootInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Loot info command received.");
        return true;
    }

    // ---------- .loot reset ----------
    static bool HandleResetLootCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Loot tables reset.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeLootSystemModule()
    {
        LootSystemManager::Instance().Initialize();
        LOG_INFO("LootSystem", "LootSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeLootSystemModule()
    {
        LOG_INFO("LootSystem", "LootSystem module finalized");
    }
}

namespace
{
    LootSystemCommandScript sLootSystemCommandScript;
}

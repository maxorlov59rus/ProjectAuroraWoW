/*
 * Economy module — implementation
 *
 * Custom economy hooks: auctions, currencies and exchanges.
 *
 * This module registers one AzerothCore script:
 *   - EconomyCommandScript : CommandScript (chat commands)
 */

#include "EconomyModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// EconomyManager — implementation
// ============================================================================

EconomyManager::EconomyManager() = default;

EconomyManager::~EconomyManager() = default;

EconomyManager& EconomyManager::Instance()
{
    static EconomyManager inst;
    return inst;
}

bool EconomyManager::Initialize()
{
    LOG_INFO("Economy", "Economy module initialized");
    return true;
}

void EconomyManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("Economy", "Player {} logged in — economy system available", p->GetName());
}

void EconomyManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;
    LOG_INFO("Economy", "Player {} leveled up", p->GetName());
}


// ============================================================================
// CommandScript
// ============================================================================

class EconomyCommandScript : public CommandScript
{
public:
    EconomyCommandScript() : CommandScript("EconomyCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable economyTable =
        {
            { "auction", HandleAuctionCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "currency", HandleCurrencyCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "exchange", HandleExchangeCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "economy", economyTable }
        };

        return commandTable;
    }

private:
    // ---------- .economy auction ----------
    static bool HandleAuctionCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Auction command received.");
        return true;
    }

    // ---------- .economy currency ----------
    static bool HandleCurrencyCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Currency command received.");
        return true;
    }

    // ---------- .economy exchange ----------
    static bool HandleExchangeCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Exchange command received.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeEconomyModule()
    {
        EconomyManager::Instance().Initialize();
        LOG_INFO("Economy", "Economy module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeEconomyModule()
    {
        LOG_INFO("Economy", "Economy module finalized");
    }
}

namespace
{
    EconomyCommandScript sEconomyCommandScript;
}

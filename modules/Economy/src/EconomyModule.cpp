/*
 * Economy module - implementation
 *
 * Custom economy hooks: auctions, currencies and exchanges.
 *
 * This module registers two AzerothCore scripts:
 *   - EconomyCommandScript : CommandScript (chat commands)
 *   - EconomyWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_economy.conf (see Economy/conf/aurora_economy.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "EconomyModule.h"

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
// EconomyManager - implementation
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
    m_enabled = Aurora::Settings::Enabled("Aurora.Economy.Enabled", "Economy");
    m_auctionDepositPercent = Aurora::Settings::Bounded<uint32_t>("Aurora.Economy.AuctionDepositPercent", 5, 0, 100, "Economy");

    if (!m_enabled)
    {
        LOG_INFO("Economy", "Economy disabled by Aurora.Economy.Enabled");
        return true;
    }

    LOG_INFO("Economy", "Economy module initialized (auction deposit {} percent)", m_auctionDepositPercent);
    return true;
}

bool EconomyManager::IsEnabled() const
{
    return m_enabled;
}

void EconomyManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("Economy", "Player {} logged in - economy system available", p->GetName());
}

void EconomyManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
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
    static bool HandleAuctionCommand(ChatHandler* handler, uint32 bid)
    {
        if (!EconomyManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("Economy is disabled by configuration.");
            return true;
        }

        uint32 const deposit = static_cast<uint32_t>(
            static_cast<uint64_t>(bid) * EconomyManager::Instance().AuctionDepositPercent() / 100);

        handler->PSendSysMessage("Auction of {} copper needs a {} copper deposit ({} percent).", bid, deposit, EconomyManager::Instance().AuctionDepositPercent());
        return true;
    }


    // ---------- .economy currency ----------
    static bool HandleCurrencyCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!EconomyManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("Economy is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Currency command received.");
        return true;
    }


    // ---------- .economy exchange ----------
    static bool HandleExchangeCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!EconomyManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("Economy is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Exchange command received.");
        return true;
    }

}

// ============================================================================
// WorldScript
// ============================================================================

class EconomyWorldScript : public WorldScript
{
public:
    EconomyWorldScript() : WorldScript("EconomyWorldScript") { }

    void OnStartup() override
    {
        EconomyManager::Instance().Initialize();
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

void AddEconomyScripts()
{
    new EconomyCommandScript();
    new EconomyWorldScript();
}

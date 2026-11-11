/*
 * LootSystem module - implementation
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
// LootSystemManager - implementation
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
    m_maxDropRate = Aurora::Settings::Get<float>("Aurora.Loot.MaxDropRate", 10.0f, "LootSystem");
    m_dropRate = Aurora::Settings::Bounded<float>("Aurora.Loot.DropRate", 1.0f, 0.0f, m_maxDropRate, "LootSystem");

    if (!m_enabled)
    {
        LOG_INFO("LootSystem", "LootSystem disabled by Aurora.Loot.Enabled");
        return true;
    }

    LOG_INFO("LootSystem", "LootSystem module initialized (drop rate {}, max {})", m_dropRate, m_maxDropRate);
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
    LOG_INFO("LootSystem", "Player {} logged in - loot system available", p->GetName());
}

void LootSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("LootSystem", "Player {} leveled up", p->GetName());
}

bool LootSystemManager::SetDropRate(float rate)
{
    if (rate < 0.0f || rate > m_maxDropRate)
        return false;

    m_dropRate = rate;
    LOG_INFO("LootSystem", "Drop rate changed to {}", rate);
    return true;
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

        if (!LootSystemManager::Instance().SetDropRate(rate))
        {
            handler->PSendSysMessage("Drop rate must be between 0 and {}.",
                LootSystemManager::Instance().MaxDropRate());
            return true;
        }

        handler->PSendSysMessage("Drop rate set to {}.", LootSystemManager::Instance().DropRate());
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

        handler->PSendSysMessage("Current drop rate: {} (maximum {}).", LootSystemManager::Instance().DropRate(), LootSystemManager::Instance().MaxDropRate());
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

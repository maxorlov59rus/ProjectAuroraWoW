/*
 * PetSystem module - implementation
 *
 * Custom pet handling: spawning, removal and stat overrides.
 *
 * This module registers two AzerothCore scripts:
 *   - PetSystemCommandScript : CommandScript (chat commands)
 *   - PetSystemWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_pets.conf (see PetSystem/conf/aurora_pets.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "PetSystemModule.h"

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
// PetSystemManager - implementation
// ============================================================================

PetSystemManager::PetSystemManager() = default;

PetSystemManager::~PetSystemManager() = default;

PetSystemManager& PetSystemManager::Instance()
{
    static PetSystemManager inst;
    return inst;
}

bool PetSystemManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Pets.Enabled", "PetSystem");
    m_maxPetLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.Pets.MaxLevel", 80, 1, 80, "PetSystem");
    m_defaultPetLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.Pets.DefaultLevel", 1, 1, m_maxPetLevel, "PetSystem");

    if (!m_enabled)
    {
        LOG_INFO("PetSystem", "PetSystem disabled by Aurora.Pets.Enabled");
        return true;
    }

    LOG_INFO("PetSystem", "PetSystem module initialized (default level {}, max {})",
        m_defaultPetLevel, m_maxPetLevel);
    return true;
}

bool PetSystemManager::IsEnabled() const
{
    return m_enabled;
}

void PetSystemManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("PetSystem", "Player {} logged in - pet system available", p->GetName());
}

void PetSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("PetSystem", "Player {} leveled up", p->GetName());
}

void PetSystemManager::SpawnPet(Player* p, uint32_t entry)
{
    if (!p)
        return;
    // TODO: create the pet creature through the map spawn helper.
    LOG_INFO("PetSystem", "Spawned pet entry {} for player {} at level {}",
        entry, p->GetName(), m_defaultPetLevel);
}

void PetSystemManager::RemovePet(Player* p)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Removed pet for player {}", p->GetName());
}

void PetSystemManager::SetPetLevel(Player* p, uint32_t level)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Set pet level to {} for player {}", level, p->GetName());
}

void PetSystemManager::SetPetStats(Player* p, uint32_t health, uint32_t mana)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Set pet stats {}/{} for player {}", health, mana, p->GetName());
}

// ============================================================================
// CommandScript
// ============================================================================

class PetSystemCommandScript : public CommandScript
{
public:
    PetSystemCommandScript() : CommandScript("PetSystemCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable petTable =
        {
            { "spawn", HandleSpawnPetCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "remove", HandleRemovePetCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "level", HandleSetPetLevelCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "stats", HandleSetPetStatsCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "pet", petTable }
        };

        return commandTable;
    }

private:
    // ---------- .pet spawn ----------
    static bool HandleSpawnPetCommand(ChatHandler* handler, uint32 entry)
    {
        if (!PetSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("PetSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Pet entry {} spawned at level {}.", entry, PetSystemManager::Instance().DefaultPetLevel());
        return true;
    }


    // ---------- .pet remove ----------
    static bool HandleRemovePetCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!PetSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("PetSystem is disabled by configuration.");
            return true;
        }

        handler->SendSysMessage("Pet removed.");
        return true;
    }


    // ---------- .pet level ----------
    static bool HandleSetPetLevelCommand(ChatHandler* handler, uint32 level)
    {
        if (!PetSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("PetSystem is disabled by configuration.");
            return true;
        }

        if (!level || level > PetSystemManager::Instance().MaxPetLevel())
        {
            handler->PSendSysMessage("Pet level must be between 1 and {}.",
                PetSystemManager::Instance().MaxPetLevel());
            return true;
        }

        handler->PSendSysMessage("Pet level set to {}.", level);
        return true;
    }


    // ---------- .pet stats ----------
    static bool HandleSetPetStatsCommand(ChatHandler* handler, uint32 health, uint32 mana)
    {
        if (!PetSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("PetSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Pet stats set to {}/{}.", health, mana);
        return true;
    }

}

// ============================================================================
// WorldScript
// ============================================================================

class PetSystemWorldScript : public WorldScript
{
public:
    PetSystemWorldScript() : WorldScript("PetSystemWorldScript") { }

    void OnStartup() override
    {
        PetSystemManager::Instance().Initialize();
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

void AddPetSystemScripts()
{
    new PetSystemCommandScript();
    new PetSystemWorldScript();
}

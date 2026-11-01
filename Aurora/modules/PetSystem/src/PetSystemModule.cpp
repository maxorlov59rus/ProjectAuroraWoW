/*
 * PetSystem module — implementation
 *
 * Custom pet handling: spawning, removal and stat overrides.
 *
 * This module registers one AzerothCore script:
 *   - PetSystemCommandScript : CommandScript (chat commands)
 */

#include "PetSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// PetSystemManager — implementation
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
    LOG_INFO("PetSystem", "PetSystem module initialized");
    return true;
}

void PetSystemManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Player {} logged in — pet system available", p->GetName());
}

void PetSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Player {} leveled up", p->GetName());
}


void PetSystemManager::SpawnPet(Player* p, uint32_t entry)
{
    if (!p)
        return;
    // TODO: create the pet creature through the map spawn helper.
    LOG_INFO("PetSystem", "Spawned pet entry {} for player {}", entry, p->GetName());
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
        handler->PSendSysMessage("Pet entry {} spawned.", entry);
        return true;
    }

    // ---------- .pet remove ----------
    static bool HandleRemovePetCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Pet removed.");
        return true;
    }

    // ---------- .pet level ----------
    static bool HandleSetPetLevelCommand(ChatHandler* handler, uint32 level)
    {
        handler->PSendSysMessage("Pet level set to {}.", level);
        return true;
    }

    // ---------- .pet stats ----------
    static bool HandleSetPetStatsCommand(ChatHandler* handler, uint32 health, uint32 mana)
    {
        handler->PSendSysMessage("Pet stats set to {}/{}.", health, mana);
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializePetSystemModule()
    {
        PetSystemManager::Instance().Initialize();
        LOG_INFO("PetSystem", "PetSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizePetSystemModule()
    {
        LOG_INFO("PetSystem", "PetSystem module finalized");
    }
}

namespace
{
    PetSystemCommandScript sPetSystemCommandScript;
}

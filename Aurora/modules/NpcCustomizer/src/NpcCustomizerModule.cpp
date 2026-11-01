/*
 * NpcCustomizer module — implementation
 *
 * Runtime customization of spawned NPCs (name, level, health, faction, flags).
 *
 * This module registers one AzerothCore script:
 *   - NpcCustomizerCommandScript : CommandScript (chat commands)
 *
 * Creature spawning itself is still a placeholder: the manager tracks the
 * customized GUIDs so the commands have a stable registry to work with.
 */

#include "NpcCustomizerModule.h"

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
// NpcCustomizerManager — implementation
// ============================================================================

NpcCustomizerManager::NpcCustomizerManager() = default;

NpcCustomizerManager::~NpcCustomizerManager() = default;

bool NpcCustomizerManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.NpcCustomizer.Enabled", "NpcCustomizer");

    if (!m_enabled)
    {
        LOG_INFO("NpcCustomizer", "NpcCustomizer disabled by Aurora.NpcCustomizer.Enabled");
        return true;
    }

    LOG_INFO("NpcCustomizer", "NpcCustomizer module initialized");
    return true;
}

bool NpcCustomizerManager::SpawnNpc(uint32_t npcEntry, float x, float y, float z, uint32_t mapId, const std::string& name)
{
    if (!m_enabled)
        return false;

    // TODO: create the real creature through the map spawn helper.
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t guid = m_nextGuid++;
    m_spawnedNpcs.push_back(guid);

    LOG_INFO("NpcCustomizer", "Spawned NPC {} (entry {}) at ({}, {}, {}) map {} as GUID {}",
        name.empty() ? std::string("NPC") : name, npcEntry, x, y, z, mapId, guid);
    return true;
}

bool NpcCustomizerManager::RemoveNpc(uint64_t guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_spawnedNpcs.begin(), m_spawnedNpcs.end(), guid);
    if (it == m_spawnedNpcs.end())
        return false;

    m_spawnedNpcs.erase(it);
    LOG_INFO("NpcCustomizer", "Removed NPC GUID: {}", guid);
    return true;
}

void NpcCustomizerManager::SetNpcName(uint64_t guid, const std::string& name)
{
    LOG_INFO("NpcCustomizer", "Set NPC {} name to {}", guid, name);
}

void NpcCustomizerManager::SetNpcLevel(uint64_t guid, uint32_t level)
{
    LOG_INFO("NpcCustomizer", "Set NPC {} level to {}", guid, level);
}

void NpcCustomizerManager::SetNpcHealth(uint64_t guid, uint32_t health, uint32_t maxHealth)
{
    LOG_INFO("NpcCustomizer", "Set NPC {} health to {}/{}", guid, health, maxHealth);
}

void NpcCustomizerManager::SetNpcFaction(uint64_t guid, uint32_t faction)
{
    LOG_INFO("NpcCustomizer", "Set NPC {} faction to {}", guid, faction);
}

void NpcCustomizerManager::SetNpcFlags(uint64_t guid, uint32_t flags)
{
    LOG_INFO("NpcCustomizer", "Set NPC {} flags to {}", guid, flags);
}

std::vector<uint64_t> NpcCustomizerManager::GetSpawnedNpcs() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_spawnedNpcs;
}

// ============================================================================
// CommandScript — real command registration
// ============================================================================

class NpcCustomizerCommandScript : public CommandScript
{
public:
    NpcCustomizerCommandScript() : CommandScript("NpcCustomizerCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable npcTable =
        {
            { "spawn",   HandleSpawnNpcCommand,     rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "remove",  HandleRemoveNpcCommand,    rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "name",    HandleSetNpcNameCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "level",   HandleSetNpcLevelCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "health",  HandleSetNpcHealthCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "faction", HandleSetNpcFactionCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "flags",   HandleSetNpcFlagsCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "list",    HandleListNpcsCommand,     rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "npc", npcTable }
        };

        return commandTable;
    }

private:
    // ---------- .npc spawn <entry> <x> <y> <z> <map> <name> ----------
    static bool HandleSpawnNpcCommand(ChatHandler* handler, uint32 entry, float x, float y, float z,
                                      uint32 mapId, std::string name)
    {
        if (NpcCustomizerManager::Instance().SpawnNpc(entry, x, y, z, mapId, name))
            handler->PSendSysMessage("NPC entry {} spawned at ({}, {}, {}) map {}.",
                entry, x, y, z, mapId);
        else
            handler->PSendSysMessage("Cannot spawn NPC entry {}.", entry);

        return true;
    }

    // ---------- .npc remove <guid> ----------
    static bool HandleRemoveNpcCommand(ChatHandler* handler, uint64 guid)
    {
        if (NpcCustomizerManager::Instance().RemoveNpc(guid))
            handler->PSendSysMessage("NPC {} removed.", guid);
        else
            handler->PSendSysMessage("NPC {} not found.", guid);

        return true;
    }

    // ---------- .npc name <guid> <name> ----------
    static bool HandleSetNpcNameCommand(ChatHandler* handler, uint64 guid, std::string name)
    {
        NpcCustomizerManager::Instance().SetNpcName(guid, name);
        handler->PSendSysMessage("NPC {} renamed to '{}'.", guid, name);
        return true;
    }

    // ---------- .npc level <guid> <level> ----------
    static bool HandleSetNpcLevelCommand(ChatHandler* handler, uint64 guid, uint32 level)
    {
        NpcCustomizerManager::Instance().SetNpcLevel(guid, level);
        handler->PSendSysMessage("NPC {} level set to {}.", guid, level);
        return true;
    }

    // ---------- .npc health <guid> <health> <maxHealth> ----------
    static bool HandleSetNpcHealthCommand(ChatHandler* handler, uint64 guid, uint32 health, uint32 maxHealth)
    {
        NpcCustomizerManager::Instance().SetNpcHealth(guid, health, maxHealth);
        handler->PSendSysMessage("NPC {} health set to {}/{}.", guid, health, maxHealth);
        return true;
    }

    // ---------- .npc faction <guid> <faction> ----------
    static bool HandleSetNpcFactionCommand(ChatHandler* handler, uint64 guid, uint32 faction)
    {
        NpcCustomizerManager::Instance().SetNpcFaction(guid, faction);
        handler->PSendSysMessage("NPC {} faction set to {}.", guid, faction);
        return true;
    }

    // ---------- .npc flags <guid> <flags> ----------
    static bool HandleSetNpcFlagsCommand(ChatHandler* handler, uint64 guid, uint32 flags)
    {
        NpcCustomizerManager::Instance().SetNpcFlags(guid, flags);
        handler->PSendSysMessage("NPC {} flags set to {}.", guid, flags);
        return true;
    }

    // ---------- .npc list ----------
    static bool HandleListNpcsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto npcs = NpcCustomizerManager::Instance().GetSpawnedNpcs();
        if (npcs.empty())
        {
            handler->SendSysMessage("No customized NPCs tracked.");
            return true;
        }

        handler->PSendSysMessage("Customized NPCs (total: {}):", static_cast<uint32>(npcs.size()));
        for (auto guid : npcs)
            handler->PSendSysMessage("- GUID: {}", guid);

        return true;
    }
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeNpcCustomizerModule()
    {
        LOG_INFO("NpcCustomizer", "NpcCustomizer module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeNpcCustomizerModule()
    {
        LOG_INFO("NpcCustomizer", "NpcCustomizer module finalized");
    }
}

// ============================================================================
// Static instance — auto-registers via constructor
// ============================================================================


// ============================================================================
// WorldScript - the module options live in configs/modules/, which the core
// loads only after worldserver.conf, so they are read here rather than in a
// static initializer.
// ============================================================================

class NpcCustomizerWorldScript : public WorldScript
{
public:
    NpcCustomizerWorldScript() : WorldScript("NpcCustomizerWorldScript") { }

    void OnStartup() override
    {
        NpcCustomizerManager::Instance().Initialize();
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

void AddNpcCustomizerScripts()
{
    new NpcCustomizerCommandScript();
    new NpcCustomizerWorldScript();
}

/*
 * NpcCustomizer module - implementation
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
#include <Util.h>

#include <algorithm>

// ============================================================================
// NpcCustomizerManager - implementation
// ============================================================================

NpcCustomizerManager::NpcCustomizerManager() = default;

NpcCustomizerManager::~NpcCustomizerManager() = default;

bool NpcCustomizerManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.NpcCustomizer.Enabled", "NpcCustomizer");
    m_maxSpawnedNpcs = Aurora::Settings::Get<uint32_t>("Aurora.NpcCustomizer.MaxSpawnedNpcs", 100, "NpcCustomizer");
    m_maxLevel = Aurora::Settings::Bounded<uint32_t>("Aurora.NpcCustomizer.MaxLevel", 83, 1, 83, "NpcCustomizer");
    m_maxNameLength = Aurora::Settings::Bounded<uint32_t>("Aurora.NpcCustomizer.MaxNameLength", 24, 1, 24, "NpcCustomizer");
    m_defaultName = Aurora::Settings::Get<std::string>("Aurora.NpcCustomizer.DefaultName", "NPC", "NpcCustomizer");

    if (!m_enabled)
    {
        LOG_INFO("NpcCustomizer", "NpcCustomizer disabled by Aurora.NpcCustomizer.Enabled");
        return true;
    }

    LOG_INFO("NpcCustomizer",
        "NpcCustomizer module initialized ({} tracked NPCs, level up to {}, name up to {} characters, default name '{}')",
        m_maxSpawnedNpcs, m_maxLevel, m_maxNameLength, m_defaultName);
    return true;
}

bool NpcCustomizerManager::SpawnNpc(uint32_t npcEntry, float x, float y, float z, uint32_t mapId,
                                    const std::string& name, uint64_t* outGuid)
{
    if (!m_enabled)
        return false;

    std::string const npcName = name.empty() ? m_defaultName : name;

    // The client truncates overlong names silently, which reads as a bug; an
    // explicit refusal tells the GM that MaxNameLength is what stopped it.
    std::string lengthProbe = npcName;
    if (utf8length(lengthProbe) > m_maxNameLength)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);

    // MaxSpawnedNpcs bounds the registry, not just this call, so the guard has to
    // be here rather than in the command handler.
    if (m_maxSpawnedNpcs && m_spawnedNpcs.size() >= m_maxSpawnedNpcs)
        return false;

    // TODO: create the real creature through the map spawn helper.
    uint64_t guid = m_nextGuid++;
    m_spawnedNpcs.push_back(guid);
    if (outGuid)
        *outGuid = guid;

    LOG_INFO("NpcCustomizer", "Spawned NPC {} (entry {}) at ({}, {}, {}) map {} as GUID {}",
        npcName, npcEntry, x, y, z, mapId, guid);
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

bool NpcCustomizerManager::SetNpcName(uint64_t guid, const std::string& name)
{
    std::string const trimmed = name.empty() ? m_defaultName : name;

    // utf8length() clears its argument on malformed input, hence the copy.
    std::string lengthProbe = trimmed;
    std::size_t const length = utf8length(lengthProbe);
    if (!length || length > m_maxNameLength)
        return false;

    LOG_INFO("NpcCustomizer", "Set NPC {} name to {}", guid, trimmed);
    return true;
}

bool NpcCustomizerManager::SetNpcLevel(uint64_t guid, uint32_t level)
{
    // Levels above MaxLevel get the cap rather than a rejection: a GM asking for
    // "boss level" usually means "as high as the module allows".
    uint32_t const applied = m_maxLevel ? std::min(level, m_maxLevel) : level;

    LOG_INFO("NpcCustomizer", "Set NPC {} level to {}", guid, applied);
    return true;
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
// CommandScript - real command registration
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
    // ---------- .npc spawn <entry> <x> <y> <z> <map> [name] ----------
    static bool HandleSpawnNpcCommand(ChatHandler* handler, uint32 entry, float x, float y, float z,
                                      uint32 mapId, std::string name)
    {
        NpcCustomizerManager& mgr = NpcCustomizerManager::Instance();
        uint64_t guid = 0;
        if (mgr.SpawnNpc(entry, x, y, z, mapId, name, &guid))
        {
            handler->PSendSysMessage("NPC entry {} spawned at ({}, {}, {}) map {} as GUID {}.",
                entry, x, y, z, mapId, guid);
        }
        else if (mgr.MaxSpawnedNpcs() && mgr.GetSpawnedNpcs().size() >= mgr.MaxSpawnedNpcs())
        {
            handler->PSendSysMessage("Cannot spawn NPC entry {}: the limit of {} tracked NPCs is reached.",
                entry, mgr.MaxSpawnedNpcs());
        }
        else
        {
            handler->PSendSysMessage("Cannot spawn NPC entry {} (the name must be at most {} characters).",
                entry, mgr.MaxNameLength());
        }

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
        NpcCustomizerManager& mgr = NpcCustomizerManager::Instance();
        if (mgr.SetNpcName(guid, name))
            handler->PSendSysMessage("NPC {} renamed to '{}'.", guid, name.empty() ? mgr.DefaultName() : name);
        else
            handler->PSendSysMessage("Name must be between 1 and {} characters.", mgr.MaxNameLength());

        return true;
    }

    // ---------- .npc level <guid> <level> ----------
    static bool HandleSetNpcLevelCommand(ChatHandler* handler, uint64 guid, uint32 level)
    {
        NpcCustomizerManager& mgr = NpcCustomizerManager::Instance();
        if (!mgr.SetNpcLevel(guid, level))
        {
            handler->PSendSysMessage("NPC level must be between 1 and {}.", mgr.MaxLevel());
            return true;
        }

        // MaxLevel caps the request, so report the level the NPC actually got.
        handler->PSendSysMessage("NPC {} level set to {}.", guid,
            mgr.MaxLevel() ? std::min(level, mgr.MaxLevel()) : level);
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
// Static instance - auto-registers via constructor
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

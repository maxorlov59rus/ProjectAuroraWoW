/*
 * NpcCustomizerModule - NPC customization module for AzerothCore (WotLK)
 *
 * Allows creating and modifying NPCs at runtime.
 */

#pragma once

#include "Define.h"
#include <string>
#include <vector>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
class Player;
class ChatHandler;

// ---------- NpcCustomizer manager ----------

class NpcCustomizerManager
{
public:
    static NpcCustomizerManager& Instance()
    {
        static NpcCustomizerManager inst;
        return inst;
    }

    // Spawns an NPC; on success outGuid (when given) receives the tracked guid.
    bool SpawnNpc(uint32_t npcEntry, float x, float y, float z, uint32_t mapId,
                  const std::string& name, uint64_t* outGuid = nullptr);
    bool RemoveNpc(uint64_t guid);
    bool SetNpcName(uint64_t guid, const std::string& name);
    bool SetNpcLevel(uint64_t guid, uint32_t level);
    void SetNpcHealth(uint64_t guid, uint32_t health, uint32_t maxHealth);
    void SetNpcFaction(uint64_t guid, uint32_t faction);
    void SetNpcFlags(uint64_t guid, uint32_t flags);
    std::vector<uint64_t> GetSpawnedNpcs() const;

    bool Initialize();
    bool IsEnabled() const { return m_enabled; }
    uint32_t MaxSpawnedNpcs() const { return m_maxSpawnedNpcs; }
    uint32_t MaxLevel() const { return m_maxLevel; }
    uint32_t MaxNameLength() const { return m_maxNameLength; }
    std::string const& DefaultName() const { return m_defaultName; }

private:
    NpcCustomizerManager();
    ~NpcCustomizerManager();
    bool m_enabled = true;
    uint32_t m_maxSpawnedNpcs = 100;
    uint32_t m_maxLevel = 83;
    uint32_t m_maxNameLength = 24;
    std::string m_defaultName = "NPC";
    uint64_t m_nextGuid = 1;
    std::vector<uint64_t> m_spawnedNpcs;
    mutable std::mutex m_mutex;
};

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeNpcCustomizerModule();
    __declspec(dllexport) void __stdcall FinalizeNpcCustomizerModule();
}

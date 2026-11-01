/*
 * NpcCustomizerModule — модуль кастомизации NPC для AzerothCore (Wotlk)
 *
 * Предоставляет возможность создания и модификации NPC.
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

    bool SpawnNpc(uint32_t npcEntry, float x, float y, float z, uint32_t mapId, const std::string& name);
    bool RemoveNpc(uint64_t guid);
    void SetNpcName(uint64_t guid, const std::string& name);
    void SetNpcLevel(uint64_t guid, uint32_t level);
    void SetNpcHealth(uint64_t guid, uint32_t health, uint32_t maxHealth);
    void SetNpcFaction(uint64_t guid, uint32_t faction);
    void SetNpcFlags(uint64_t guid, uint32_t flags);
    std::vector<uint64_t> GetSpawnedNpcs() const;

    bool Initialize();
    bool IsEnabled() const { return m_enabled; }

private:
    NpcCustomizerManager();
    ~NpcCustomizerManager();
    bool m_enabled = true;
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

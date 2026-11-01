/*
 * NpcCustomizerModule — модуль кастомизации NPC для AzerothCore (Wotlk)
 *
 * Предоставляет возможность создания и модификации NPC.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
struct WorldSession;
struct Player;
struct ChatHandler;
struct GameObject;

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

private:
    NpcCustomizerManager() {}
    ~NpcCustomizerManager() {}
    std::vector<uint64_t> m_spawnedNpcs;
    std::mutex m_mutex;
};

// ---------- Command declarations ----------
void RegisterNpcCustomizerCommands();
bool HandleSpawnNpcCommand(ChatHandler* handler, const std::string& args);
bool HandleRemoveNpcCommand(ChatHandler* handler, const std::string& args);
bool HandleSetNpcNameCommand(ChatHandler* handler, const std::string& args);
bool HandleSetNpcLevelCommand(ChatHandler* handler, const std::string& args);
bool HandleSetNpcHealthCommand(ChatHandler* handler, const std::string& args);
bool HandleSetNpcFactionCommand(ChatHandler* handler, const std::string& args);
bool HandleSetNpcFlagsCommand(ChatHandler* handler, const std::string& args);
bool HandleListNpcsCommand(ChatHandler* handler, const std::string& args);

// ---------- Hook registrations ----------
void OnPlayerLogin(Player* pPlayer);
void OnPlayerLevelUp(Player* pPlayer);

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeNpcCustomizerModule();
    __declspec(dllexport) void __stdcall FinalizeNpcCustomizerModule();
}
#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler; class GameObject; class Creature;

// ---------- NPC Data ----------
struct CampaignNpc
{
    uint32_t entry;
    std::string name;
    std::string title;
    uint32_t level;
    uint32_t faction;
    float x, y, z;
    uint32_t mapId;
    uint32_t spawnTime;
    std::string dialogues; // Lua script name for dialogues
    bool isBoss;
    bool isVendor;
};

// ---------- Dialogue System ----------
struct DialogueNode
{
    uint32_t id;
    std::string text;
    std::vector<uint32_t> nextNodes;
    std::string luaScript;
    bool isEnd;
};

// ---------- Quest Data ----------
struct CampaignQuest
{
    uint32_t questId;
    std::string title;
    std::string description;
    uint32_t level;
    uint32_t rewardGold;
    std::string luaScript;
};

// ---------- Boss Data ----------
struct CampaignBoss
{
    uint32_t entry;
    std::string name;
    uint32_t level;
    uint32_t health;
    std::string abilities; // Lua script for boss abilities
    uint32_t spawnLocationX, spawnLocationY, spawnLocationZ;
    uint32_t mapId;
};

// ---------- Shop Data ----------
struct CampaignShop
{
    uint32_t vendorEntry;
    std::string name;
    std::vector<uint32_t> items; // item entries
    std::string luaScript;
};

// ---------- Campaign Manager ----------
class CampaignOfDarknessManager
{
public:
    static CampaignOfDarknessManager& Instance()
    {
        static CampaignOfDarknessManager inst;
        return inst;
    }

    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    
    // NPC management
    bool AddNpc(const CampaignNpc& npc);
    bool RemoveNpc(uint32_t entry);
    std::vector<CampaignNpc> GetNpcs() const;
    
    // Dialogue management
    bool AddDialogue(uint32_t npcEntry, const DialogueNode& node);
    bool ShowDialogue(Player* p, uint32_t npcEntry, uint32_t nodeId);
    
    // Quest management
    bool AddQuest(const CampaignQuest& quest);
    bool StartQuest(Player* p, uint32_t questId);
    bool CompleteQuest(Player* p, uint32_t questId);
    
    // Boss management
    bool SpawnBoss(uint32_t bossEntry);
    bool RemoveBoss(uint32_t bossEntry);
    std::vector<CampaignBoss> GetBosses() const;
    
    // Shop management
    bool AddShop(const CampaignShop& shop);
    bool ShowShop(Player* p, uint32_t vendorEntry);
    
    // Spawn list management
    bool LoadSpawnList(const std::string& filename);
    bool SaveSpawnList(const std::string& filename);

    bool IsEnabled() const { return m_enabled; }
    // Access rules read from aurora_campaign_darkness.conf: the campaign is a
    // horde-only chain, so a non-horde character has to be turned away, and the
    // level gate is what keeps low level characters out of it.
    bool RequireHorde() const { return m_requireHorde; }
    uint32_t MinLevelToUnlock() const { return m_minLevelToUnlock; }
    // Faction id given to campaign NPCs that do not carry one of their own.
    uint32_t DefaultFaction() const { return m_defaultFaction; }
    // Multiplier applied to the gold reward of a completed campaign quest.
    float RewardGoldMultiplier() const { return m_rewardGoldMultiplier; }
    // Gold a quest pays out after RewardGoldMultiplier was applied.
    uint32_t RewardGoldFor(const CampaignQuest& quest) const;

private:
    bool m_enabled = true;
    bool m_requireHorde = true;
    uint32_t m_minLevelToUnlock = 10;
    uint32_t m_defaultFaction = 2142;
    float m_rewardGoldMultiplier = 1.0f;

    CampaignOfDarknessManager() {}
    ~CampaignOfDarknessManager() {}
    
    std::map<uint32_t, CampaignNpc> m_npcs;
    std::map<uint32_t, std::map<uint32_t, DialogueNode>> m_dialogues; // npcEntry -> dialogueId -> node
    std::map<uint32_t, CampaignQuest> m_quests;
    std::map<uint32_t, CampaignBoss> m_bosses;
    std::map<uint32_t, CampaignShop> m_shops;
    mutable std::mutex m_mutex;
};

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfDarknessModule();
    __declspec(dllexport) void __stdcall FinalizeCampaignOfDarknessModule();
}
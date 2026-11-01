#pragma once

// AzerothCore's integral typedefs (uint8/uint32/...) and DLL export macros.
#include "Define.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

class Player;
// ---------- NPC Data ----------
struct CampaignNpc
{
    uint32 entry = 0;
    std::string name;
    std::string title;
    uint8 level = 1;
    uint32 health = 0;
    float x = 0.f, y = 0.f, z = 0.f;
    uint32 mapId = 0;
    std::string dialoguesScript;
    bool isBoss = false;
    bool isVendor = false;
};

// ---------- Dialogue System ----------
struct DialogueNode
{
    uint32 id = 0;
    std::string text;
    std::vector<uint32> nextNodes;
    std::string luaScript;
    bool isEnd = false;
};

// ---------- Quest Data ----------
struct CampaignQuest
{
    uint32 questId = 0;
    std::string title;
    std::string description;
    uint8 level = 1;
    uint32 rewardGold = 0;
    uint32 rewardXp = 0;
    std::string rewardItems;
    std::string luaScript;
    uint32 startNpcEntry = 0;
    uint32 endNpcEntry = 0;
};

// ---------- Boss Data ----------
struct CampaignBoss
{
    uint32 entry = 0;
    std::string name;
    std::string title;
    uint8 level = 80;
    uint32 health = 0;
    uint32 mana = 0;
    std::string abilities;
    std::string spawnScript;
    float x = 0.f, y = 0.f, z = 0.f;
    uint32 mapId = 0;
    uint32 respawnTime = 3600;
};

// ---------- Shop Data ----------
struct CampaignShop
{
    uint32 vendorEntry = 0;
    std::string name;
    std::string items;
    std::string luaScript;
    float discountHorde = 0.f;
    float discountAlliance = 0.f;
};

// ============================================================================
// CampaignOfLightManager - shared state (single instance)
// ============================================================================

class CampaignOfLightManager
{
public:
    static CampaignOfLightManager& Instance()
    {
        static CampaignOfLightManager inst;
        return inst;
    }

    CampaignOfLightManager(CampaignOfLightManager const&) = delete;
    CampaignOfLightManager& operator=(CampaignOfLightManager const&) = delete;

    bool Initialize();

    // Data is loaded lazily: the CommandScript/PlayerScript instances are
    // registered while the core starts up, before the module entry points run.
    void EnsureLoaded();
    bool IsLoaded() const { return m_loaded; }

    // NPC
    void LoadNpcsFromDb();
    bool AddNpc(CampaignNpc const& npc);
    bool RemoveNpc(uint32 entry);
    CampaignNpc const* GetNpc(uint32 entry) const;
    std::vector<CampaignNpc> GetNpcs() const;

    // Dialogue
    bool AddDialogue(uint32 npcEntry, DialogueNode const& node);
    bool ShowDialogue(Player* player, uint32 npcEntry, uint32 nodeId);

    // Quest
    void LoadQuestsFromDb();
    bool AddQuest(CampaignQuest const& quest);
    bool StartQuest(Player* player, uint32 questId);
    bool CompleteQuest(Player* player, uint32 questId);
    CampaignQuest const* GetQuest(uint32 questId) const;

    // Boss
    void LoadBossesFromDb();
    bool SpawnBoss(uint32 bossEntry, float x, float y, float z, uint32 mapId);
    bool RemoveBoss(uint32 bossEntry);
    std::vector<CampaignBoss> GetBosses() const;

    // Shop
    void LoadShopsFromDb();
    bool AddShop(CampaignShop const& shop);
    bool ShowShop(Player* player, uint32 vendorEntry);

    bool IsEnabled() const { return m_enabled; }

    // Options from aurora_campaign_light.conf. They are not decoration: Faction
    // and IncludeNeutral build the WHERE clause of the loader queries,
    // RequireAlliance/MinLevelToUnlock gate StartQuest and BossRespawnMultiplier
    // scales the respawn timer of the campaign bosses.
    std::string const& Faction() const { return m_faction; }
    bool IncludeNeutral() const { return m_includeNeutral; }
    bool RequireAlliance() const { return m_requireAlliance; }
    uint32 MinLevelToUnlock() const { return m_minLevelToUnlock; }
    float BossRespawnMultiplier() const { return m_bossRespawnMultiplier; }
    // Respawn time of a boss after BossRespawnMultiplier was applied.
    uint32 RespawnTimeFor(CampaignBoss const& boss) const;
    // WHERE fragment limiting the campaign_* queries to the configured factions.
    std::string FactionCondition(std::string const& column) const;

private:
    bool m_enabled = true;
    std::string m_faction = "alliance";
    bool m_includeNeutral = true;
    bool m_requireAlliance = true;
    uint32 m_minLevelToUnlock = 10;
    float m_bossRespawnMultiplier = 1.0f;

    CampaignOfLightManager() = default;
    ~CampaignOfLightManager() = default;

    std::map<uint32, CampaignNpc> m_npcs;
    std::map<uint32, std::map<uint32, DialogueNode>> m_dialogues;
    std::map<uint32, CampaignQuest> m_quests;
    std::map<uint32, CampaignBoss> m_bosses;
    std::map<uint32, CampaignShop> m_shops;
    mutable std::mutex m_mutex;
    std::once_flag m_loadFlag;
    bool m_loaded = false;

    void LoadAllFromDb();
};

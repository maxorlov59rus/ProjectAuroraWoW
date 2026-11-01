#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

class Player;
class ChatHandler;

// ---------- NPC Data ----------
struct CampaignNpc
{
    uint32 entry;
    std::string name;
    std::string title;
    uint8 level;
    uint32 health;
    float x, y, z;
    uint32 mapId;
    std::string dialoguesScript;
    bool isBoss;
    bool isVendor;
};

// ---------- Dialogue System ----------
struct DialogueNode
{
    uint32 id;
    std::string text;
    std::vector<uint32> nextNodes;
    std::string luaScript;
    bool isEnd;
};

// ---------- Quest Data ----------
struct CampaignQuest
{
    uint32 questId;
    std::string title;
    std::string description;
    uint8 level;
    uint32 rewardGold;
    uint32 rewardXp;
    std::string rewardItems;
    std::string luaScript;
    uint32 startNpcEntry;
    uint32 endNpcEntry;
};

// ---------- Boss Data ----------
struct CampaignBoss
{
    uint32 entry;
    std::string name;
    std::string title;
    uint8 level;
    uint32 health;
    uint32 mana;
    std::string abilities;
    std::string spawnScript;
    float x, y, z;
    uint32 mapId;
    uint32 respawnTime;
};

// ---------- Shop Data ----------
struct CampaignShop
{
    uint32 vendorEntry;
    std::string name;
    std::string items; // comma-separated item entries
    std::string luaScript;
    float discountHorde;
    float discountAlliance;
};

// ---------- Campaign Manager ----------
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
    void OnPlayerLogin(Player* player);
    void OnPlayerLevelUp(Player* player, uint8 newLevel);

    // NPC management
    void LoadNpcsFromDb();
    bool AddNpc(CampaignNpc const& npc);
    bool RemoveNpc(uint32 entry);
    CampaignNpc const* GetNpc(uint32 entry) const;
    std::vector<CampaignNpc> GetNpcs() const;

    // Dialogue management
    bool AddDialogue(uint32 npcEntry, DialogueNode const& node);
    bool ShowDialogue(Player* player, uint32 npcEntry, uint32 nodeId);

    // Quest management
    void LoadQuestsFromDb();
    bool AddQuest(CampaignQuest const& quest);
    bool StartQuest(Player* player, uint32 questId);
    bool CompleteQuest(Player* player, uint32 questId);
    CampaignQuest const* GetQuest(uint32 questId) const;

    // Boss management
    void LoadBossesFromDb();
    bool SpawnBoss(uint32 bossEntry, float x, float y, float z, uint32 mapId);
    bool RemoveBoss(uint32 bossEntry);
    std::vector<CampaignBoss> GetBosses() const;

    // Shop management
    void LoadShopsFromDb();
    bool AddShop(CampaignShop const& shop);
    bool ShowShop(Player* player, uint32 vendorEntry);

private:
    CampaignOfLightManager();
    ~CampaignOfLightManager() = default;

    std::map<uint32, CampaignNpc> m_npcs;
    std::map<uint32, std::map<uint32, DialogueNode>> m_dialogues;
    std::map<uint32, CampaignQuest> m_quests;
    std::map<uint32, CampaignBoss> m_bosses;
    std::map<uint32, CampaignShop> m_shops;
    std::mutex m_mutex;
};

// ---------- Command registration ----------
void RegisterCampaignOfLightCommands();

bool HandleStartCampaignCommand(ChatHandler* handler, char const* args);
bool HandleSpawnNpcCommand(ChatHandler* handler, char const* args);
bool HandleSpawnBossCommand(ChatHandler* handler, char const* args);
bool HandleShowDialogueCommand(ChatHandler* handler, char const* args);
bool HandleListNpcsCommand(ChatHandler* handler, char const* args);
bool HandleListBossesCommand(ChatHandler* handler, char const* args);
bool HandleShowShopCommand(ChatHandler* handler, char const* args);
bool HandleCampaignInfoCommand(ChatHandler* handler, char const* args);

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfLightModule();
    __declspec(dllexport) void __stdcall FinalizeCampaignOfLightModule();
}

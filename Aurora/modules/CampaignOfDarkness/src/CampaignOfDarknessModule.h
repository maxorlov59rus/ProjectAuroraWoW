#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler; struct GameObject; struct Creature;

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

private:
    CampaignOfDarknessManager() {}
    ~CampaignOfDarknessManager() {}
    
    std::map<uint32_t, CampaignNpc> m_npcs;
    std::map<uint32_t, std::map<uint32_t, DialogueNode>> m_dialogues; // npcEntry -> dialogueId -> node
    std::map<uint32_t, CampaignQuest> m_quests;
    std::map<uint32_t, CampaignBoss> m_bosses;
    std::map<uint32_t, CampaignShop> m_shops;
    std::mutex m_mutex;
};

// ---------- Command declarations ----------
void RegisterCampaignOfDarknessCommands();
bool HandleStartCampaignCommand(ChatHandler* handler, const std::string& args);
bool HandleSpawnNpcCommand(ChatHandler* handler, const std::string& args);
bool HandleSpawnBossCommand(ChatHandler* handler, const std::string& args);
bool HandleShowDialogueCommand(ChatHandler* handler, const std::string& args);
bool HandleListNpcsCommand(ChatHandler* handler, const std::string& args);
bool HandleListBossesCommand(ChatHandler* handler, const std::string& args);
bool HandleShowShopCommand(ChatHandler* handler, const std::string& args);
bool HandleCampaignInfoCommand(ChatHandler* handler, const std::string& args);

// ---------- Hook registrations ----------
void OnPlayerLogin(Player* pPlayer);
void OnPlayerLevelUp(Player* pPlayer);

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfDarknessModule();
    __declspec(dllexport) void __stdcall FinalizeCampaignOfDarknessModule();
}
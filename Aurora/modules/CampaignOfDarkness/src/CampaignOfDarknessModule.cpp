#include "CampaignOfDarknessModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

CampaignOfDarknessManager::CampaignOfDarknessManager() {}
CampaignOfDarknessManager::~CampaignOfDarknessManager() {}

bool CampaignOfDarknessManager::Initialize()
{
    LOG_INFO("CampaignOfDarkness", "Campaign of Darkness module initialized");
    return true;
}

void CampaignOfDarknessManager::OnPlayerLogin(Player* pPlayer)
{
    LOG_INFO("CampaignOfDarkness", "Player %s logged in - Campaign of Darkness available", pPlayer->GetName().c_str());
}

void CampaignOfDarknessManager::OnPlayerLevelUp(Player* pPlayer)
{
    LOG_INFO("CampaignOfDarkness", "Player %s leveled up", pPlayer->GetName().c_str());
}

// ---------- NPC management ----------

bool CampaignOfDarknessManager::AddNpc(const CampaignNpc& npc)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_npcs[npc.entry] = npc;
    LOG_INFO("CampaignOfDarkness", "Added NPC %s (entry %u)", npc.name.c_str(), npc.entry);
    return true;
}

bool CampaignOfDarknessManager::RemoveNpc(uint32_t entry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_npcs.find(entry);
    if (it == m_npcs.end()) return false;
    m_npcs.erase(it);
    LOG_INFO("CampaignOfDarkness", "Removed NPC entry %u", entry);
    return true;

// ---------- Dialogue management ----------

bool CampaignOfDarknessManager::AddDialogue(uint32_t npcEntry, const DialogueNode& node)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dialogues[npcEntry][node.id] = node;
    LOG_INFO("CampaignOfDarkness", "Added dialogue node %u for NPC %u", node.id, npcEntry);
    return true;
}

bool CampaignOfDarknessManager::ShowDialogue(Player* pPlayer, uint32_t npcEntry, uint32_t nodeId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto npcIt = m_dialogues.find(npcEntry);
    if (npcIt == m_dialogues.end()) return false;
    
    auto nodeIt = npcIt->second.find(nodeId);
    if (nodeIt == npcIt->second.end()) return false;
    
    const DialogueNode& node = nodeIt->second;
    LOG_INFO("CampaignOfDarkness", "Showing dialogue %u for player %s", nodeId, pPlayer->GetName().c_str());
    
    // TODO: Send dialogue packet to client
    // pPlayer->GetSession()->SendDialogue(npcEntry, node.text, node.nextNodes);
    
    return true;
}

// ---------- Quest management ----------

bool CampaignOfDarknessManager::AddQuest(const CampaignQuest& quest)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_quests[quest.questId] = quest;
    LOG_INFO("CampaignOfDarkness", "Added quest %s (ID: %u)", quest.title.c_str(), quest.questId);
    return true;
}

bool CampaignOfDarknessManager::StartQuest(Player* pPlayer, uint32_t questId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    
    const CampaignQuest& quest = it->second;

// ---------- Command registrations ----------

void RegisterCampaignOfDarknessCommands()
{
    LOG_INFO("CampaignOfDarkness", "Campaign commands registered: .startcampaign, .spawnnpc, .spawnboss, .showdialogue, .listnpcs, .listbosses, .showshop, .campaigninfo");
}


bool HandleStartCampaignCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Campaign started (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Start campaign command: %s", args.c_str());
    return true;
}

bool HandleSpawnNpcCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Spawn NPC command received (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Spawn NPC command: %s", args.c_str());
    return true;
}

bool HandleSpawnBossCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Spawn boss command received (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Spawn boss command: %s", args.c_str());
    return true;
}

bool HandleShowDialogueCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Show dialogue command received (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Show dialogue command: %s", args.c_str());
    return true;
}

bool HandleListNpcsCommand(ChatHandler* handler, const std::string& args)
{
    auto npcs = CampaignOfDarknessManager::Instance().GetNpcs();
    if (npcs.empty()) {
        handler->SendSysMessage("NPCs не найдены.");
        return true;
    }
    handler->SendSysMessage("NPCs (всего: %d):", (int)npcs.size());
    for (const auto& npc : npcs) {
        handler->SendSysMessage("- %s (entry: %u, level: %u)", npc.name.c_str(), npc.entry, npc.level);
    }
    return true;
}

bool HandleListBossesCommand(ChatHandler* handler, const std::string& args)
{
    auto bosses = CampaignOfDarknessManager::Instance().GetBosses();
    if (bosses.empty()) {
        handler->SendSysMessage("Боссы не найдены.");
        return true;
    }
    handler->SendSysMessage("Боссы (всего: %d):", (int)bosses.size());
    for (const auto& boss : bosses) {
        handler->SendSysMessage("- %s (entry: %u, level: %u)", boss.name.c_str(), boss.entry, boss.level);
    }
    return true;
}

bool HandleShowShopCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Show shop command received (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Show shop command: %s", args.c_str());
    return true;
}

bool HandleCampaignInfoCommand(ChatHandler* handler, const std::string& args)
{
    handler->SendSysMessage("Campaign info command received (placeholder)");
    LOG_INFO("CampaignOfDarkness", "Campaign info command: %s", args.c_str());
    return true;
}

// ---------- Hook registrations ----------

void OnPlayerLogin(Player* pPlayer)
{
    LOG_INFO("CampaignOfDarkness", "Player %s logged in - Campaign of Darkness available", pPlayer->GetName().c_str());
}

void OnPlayerLevelUp(Player* pPlayer)
{
    LOG_INFO("CampaignOfDarkness", "Player %s leveled up", pPlayer->GetName().c_str());
}

// ---------- Module registration ----------

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfDarknessModule()
    {
        CampaignOfDarknessManager::Instance().Initialize();
        RegisterCampaignOfDarknessCommands();
        LOG_INFO("CampaignOfDarkness", "Campaign of Darkness module initialized");
    }
    
    __declspec(dllexport) void __stdcall FinalizeCampaignOfDarknessModule()
    {
        LOG_INFO("CampaignOfDarkness", "Campaign of Darkness module finalized");
    }
}

    LOG_INFO("CampaignOfDarkness", "Player %s started quest %s (ID: %u)", pPlayer->GetName().c_str(), quest.title.c_str(), questId);
    
    // TODO: Actually give quest to player
    // pPlayer->AddQuest(&quest);
    
    return true;
}


// ---------- Boss management ----------

bool CampaignOfDarknessManager::SpawnBoss(uint32_t bossEntry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bosses.find(bossEntry);
    if (it == m_bosses.end()) return false;
    
    const CampaignBoss& boss = it->second;
    LOG_INFO("CampaignOfDarkness", "Spawning boss %s (entry %u)", boss.name.c_str(), boss.entry);
    
    // TODO: Actually spawn boss in world
    // CreatureInfo* creatureInfo = CreatureInfoStorage->LookupEntry(boss.entry);
    // if (creatureInfo) {
    //     // Spawn at boss location
    // }
    
    return true;
}

bool CampaignOfDarknessManager::RemoveBoss(uint32_t bossEntry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bosses.find(bossEntry);
    if (it == m_bosses.end()) return false;
    m_bosses.erase(it);
    LOG_INFO("CampaignOfDarkness", "Removed boss entry %u", bossEntry);
    return true;
}

std::vector<CampaignBoss> CampaignOfDarknessManager::GetBosses() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<CampaignBoss> list;
    for (const auto& pair : m_bosses) {
        list.push_back(pair.second);
    }
    return list;
}

// ---------- Shop management ----------

bool CampaignOfDarknessManager::AddShop(const CampaignShop& shop)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shops[shop.vendorEntry] = shop;
    LOG_INFO("CampaignOfDarkness", "Added shop %s (vendor %u)", shop.name.c_str(), shop.vendorEntry);
    return true;
}

bool CampaignOfDarknessManager::ShowShop(Player* pPlayer, uint32_t vendorEntry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_shops.find(vendorEntry);
    if (it == m_shops.end()) return false;
    
    const CampaignShop& shop = it->second;
    LOG_INFO("CampaignOfDarkness", "Showing shop %s to player %s", shop.name.c_str(), pPlayer->GetName().c_str());
    
    // TODO: Actually show vendor window to player
    // pPlayer->GetSession()->SendVendorWindow(vendorEntry, shop.items);
    
    return true;
}

// ---------- Spawn list management ----------

bool CampaignOfDarknessManager::LoadSpawnList(const std::string& filename)
{
    LOG_INFO("CampaignOfDarkness", "Loading spawn list from %s", filename.c_str());
    // TODO: Read from SQL or Lua file
    return true;
}

bool CampaignOfDarknessManager::SaveSpawnList(const std::string& filename)
{
    LOG_INFO("CampaignOfDarkness", "Saving spawn list to %s", filename.c_str());
    // TODO: Write to SQL or Lua file
    return true;
}

// ---------- Command registrations ----------

void RegisterCampaignOfDarknessCommands()
{
    LOG_INFO("CampaignOfDarkness", "Campaign commands registered: .startcampaign, .spawnnpc, .spawnboss, .showdialogue, .listnpcs, .listbosses, .showshop, .campaigninfo");
}

bool CampaignOfDarknessManager::CompleteQuest(Player* pPlayer, uint32_t questId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    
    const CampaignQuest& quest = it->second;
    LOG_INFO("CampaignOfDarkness", "Player %s completed quest %s (ID: %u)", pPlayer->GetName().c_str(), quest.title.c_str(), questId);
    
    // TODO: Give rewards
    // pPlayer->AddMoney(quest.rewardGold);
    // pPlayer->AddItem(quest.rewardItems);
    
    return true;
}
}

std::vector<CampaignNpc> CampaignOfDarknessManager::GetNpcs() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<CampaignNpc> list;
    for (const auto& pair : m_npcs) {
        list.push_back(pair.second);
    }
    return list;
}
// ---------- Command registrations ----------

void RegisterCampaignOfDarknessCommands()
{
    LOG_INFO("CampaignOfDarkness", "Campaign commands registered: .startcampaign, .stopevent, .spawnnpc, .listnpcs, .listbosses, .showshop, .campaigninfo");
}

bool HandleStartEventCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .startevent <минуты> <максимум_зомби>");
        return true;
    }

    uint32_t durationMinutes = uint32_t(std::stoul(tokens[1]));
    uint32_t maxZombies = (tokens.size() > 2) ? uint32_t(std::stoul(tokens[2])) : 50;

    if (CampaignOfDarknessManager::Instance().StartEvent(durationMinutes, maxZombies)) {
        handler->SendSysMessage("Апокалипсис зомби начат на %d минут с максимумом %d зомби", durationMinutes, maxZombies);
    } else {
        handler->SendSysMessage("Не удалось начать событие (возможно, оно уже запущено).");
    }
    return true;
}

bool HandleStopEventCommand(ChatHandler* handler, const std::string& args)
{
    CampaignOfDarknessManager::Instance().StopEvent();
    handler->SendSysMessage("Апокалипсис зомби остановлен");
    return true;
}

bool HandleSpawnNpcCommand(ChatHandler* handler, const std::string& args)
{
    if (CampaignOfDarknessManager::Instance().SpawnRandomZombie()) {
        handler->SendSysMessage("Зомби спавнен в случайной локации");
    } else {
        handler->SendSysMessage("Не удалось спавн зомби (событие неактивно или предел достигнут)");
    }
    return true;
}

bool HandleListNpcsCommand(ChatHandler* handler, const std::string& args)
{
    auto zombies = CampaignOfDarknessManager::Instance().GetLivingZombies();
    if (zombies.empty()) {
        handler->SendSysMessage("Зомби не spawn'ены или событие неактивно.");
        return true;
    }

    handler->SendSysMessage("Зомби (всего: %d):", (int)zombies.size());
    for (const auto& zombie : zombies) {
        handler->SendSysMessage("- %s, уровень %u, здоровье %u/%u, локация: %s",
            zombie.name.c_str(), zombie.level, zombie.health, zombie.maxHealth, zombie.spawnedLocation.c_str());
    }
    return true;
}

bool HandleZombieHealthCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 1) {
        handler->SendSysMessage("Использование: .zombiehealth <guid>");
        return true;
    }

    uint64_t guid = 0;
    try { guid = std::stoull(tokens[1]); } catch (...) { guid = 0; }

    ZombieInfo info;
    if (CampaignOfDarknessManager::Instance().GetZombieInfo(guid, info)) {
        handler->SendSysMessage("Зомби: %s, уровень %u, здоровье %u/%u, спавн: %s",
            info.name.c_str(), info.level, info.health, info.maxHealth, info.spawnedLocation.c_str());
    } else {
        handler->SendSysMessage("Зомби с GUID %llu не найден", (unsigned long long)guid);
    }
    return true;
}

// ---------- Hook registrations ----------

void OnPlayerLogin(Player* pPlayer)
{
    if (IsEventActive())
    {
        time_t remaining = GetRemainingTime();
        int minutes = remaining > 0 ? (int)(remaining / 60) : 0;
        LOG_INFO("CampaignOfDarkness", "Player %s logged in during active apocalypse event (%d minutes remaining)", pPlayer->GetName().c_str(), minutes);
    }

void OnPlayerLevelUp(Player* pPlayer)
{
    if (IsEventActive())
    {
        LOG_INFO("CampaignOfDarkness", "Player %s leveled up during apocalypse event", pPlayer->GetName().c_str());
    }
}

// ---------- Utility functions ----------

uint32_t GetZombieLevelForPlayer(Player* pPlayer)
{
    if (!pPlayer) return 80;
    return std::min(80, pPlayer->getLevel() + 5);
}

float RandomFloat(float min, float max)
{
    return min + static_cast<float>(rand() % (int)(max - min + 1));
}

// ---------- Module registration ----------

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfDarknessModule()
    {
        RegisterCampaignOfDarknessCommands();
        LOG_INFO("CampaignOfDarkness", "Campaign of Darkness module initialized");
    }
    
    __declspec(dllexport) void __stdcall FinalizeCampaignOfDarknessModule()
    {
        LOG_INFO("CampaignOfDarkness", "Campaign of Darkness module finalized");
    }
}

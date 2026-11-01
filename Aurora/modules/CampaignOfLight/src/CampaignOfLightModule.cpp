/*
 * CampaignOfLight module — implementation
 *
 * Alliance campaign content: NPCs, dialogues, quests, bosses, shops.
 * Counterpart to CampaignOfDarkness (Horde campaign).
 */

#include "CampaignOfLightModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Map.h>
#include <Creature.h>
#include <DatabaseEnv.h>

// ============================================================================
// CampaignOfLightManager
// ============================================================================

CampaignOfLightManager::CampaignOfLightManager() = default;

bool CampaignOfLightManager::Initialize()
{
    LOG_INFO("CampaignOfLight", "Loading Campaign of Light module data from database…");
    LoadNpcsFromDb();
    LoadQuestsFromDb();
    LoadBossesFromDb();
    LoadShopsFromDb();
    RegisterCampaignOfLightCommands();
    LOG_INFO("CampaignOfLight", "Campaign of Light module initialized ({} NPCs, {} quests, {} bosses, {} shops)",
        static_cast<int>(m_npcs.size()), static_cast<int>(m_quests.size()),
        static_cast<int>(m_bosses.size()), static_cast<int>(m_shops.size()));
    return true;
}

void CampaignOfLightManager::OnPlayerLogin(Player* player)
{
    if (!player)
        return;

    LOG_INFO("CampaignOfLight", "Player {} (guid {}) logged in — Campaign of Light available",
        player->GetName(), player->GetGUID().GetRawValue());
}

void CampaignOfLightManager::OnPlayerLevelUp(Player* player, uint8 newLevel)
{
    if (!player)
        return;

    LOG_INFO("CampaignOfLight", "Player {} leveled up to {}", player->GetName(), newLevel);
}

// ---------- NPC ----------

void CampaignOfLightManager::LoadNpcsFromDb()
{
    auto result = WorldDatabase.Query(
        "SELECT entry, name, title, level, health, float_x, float_y, float_z, "
        "map_id, dialogues_script, is_boss, is_vendor "
        "FROM campaign_npcs WHERE faction = 'alliance' OR faction = 'neutral'");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        CampaignNpc npc{};
        npc.entry           = fields[0].Get<uint32>();
        npc.name            = fields[1].Get<std::string>();
        npc.title           = fields[2].Get<std::string>();
        npc.level           = static_cast<uint8>(fields[3].Get<uint32>());
        npc.health          = fields[4].Get<uint32>();
        npc.x               = fields[5].Get<float>();
        npc.y               = fields[6].Get<float>();
        npc.z               = fields[7].Get<float>();
        npc.mapId           = fields[8].Get<uint32>();
        npc.dialoguesScript = fields[9].Get<std::string>();
        npc.isBoss          = fields[10].Get<bool>();
        npc.isVendor        = fields[11].Get<bool>();

        m_npcs[npc.entry] = npc;
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral NPCs", count);
}

bool CampaignOfLightManager::AddNpc(CampaignNpc const& npc)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_npcs[npc.entry] = npc;
    LOG_INFO("CampaignOfLight", "Added NPC {} (entry {})", npc.name, npc.entry);
    return true;
}

bool CampaignOfLightManager::RemoveNpc(uint32 entry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_npcs.erase(entry) > 0;
}

CampaignNpc const* CampaignOfLightManager::GetNpc(uint32 entry) const
{
    auto it = m_npcs.find(entry);
    return it != m_npcs.end() ? &it->second : nullptr;
}

std::vector<CampaignNpc> CampaignOfLightManager::GetNpcs() const
{
    std::vector<CampaignNpc> list;
    list.reserve(m_npcs.size());
    for (auto const& [_, npc] : m_npcs)
        list.push_back(npc);
    return list;
}

// ---------- Dialogue ----------

bool CampaignOfLightManager::AddDialogue(uint32 npcEntry, DialogueNode const& node)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dialogues[npcEntry][node.id] = node;
    LOG_INFO("CampaignOfLight", "Added dialogue node {} for NPC {}", node.id, npcEntry);
    return true;
}

bool CampaignOfLightManager::ShowDialogue(Player* player, uint32 npcEntry, uint32 nodeId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto npcIt = m_dialogues.find(npcEntry);
    if (npcIt == m_dialogues.end())
        return false;

    auto nodeIt = npcIt->second.find(nodeId);
    if (nodeIt == npcIt->second.end())
        return false;

    DialogueNode const& node = nodeIt->second;
    LOG_INFO("CampaignOfLight", "Showing dialogue {} for player {} (NPC {})",
        nodeId, player->GetName(), npcEntry);

    // TODO: Send dialogue packet to client
    // player->SendDirectMessage(node.text);
    return true;
}

// ---------- Quest ----------

void CampaignOfLightManager::LoadQuestsFromDb()
{
    auto result = WorldDatabase.Query(
        "SELECT quest_id, title, description, level, reward_gold, reward_xp, "
        "reward_items, lua_script, start_npc_entry, end_npc_entry "
        "FROM campaign_quests WHERE faction = 'alliance'");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        CampaignQuest quest{};
        quest.questId       = fields[0].Get<uint32>();
        quest.title         = fields[1].Get<std::string>();
        quest.description   = fields[2].Get<std::string>();
        quest.level         = static_cast<uint8>(fields[3].Get<uint32>());
        quest.rewardGold    = fields[4].Get<uint32>();
        quest.rewardXp      = fields[5].Get<uint32>();
        quest.rewardItems   = fields[6].Get<std::string>();
        quest.luaScript     = fields[7].Get<std::string>();
        quest.startNpcEntry = fields[8].Get<uint32>();
        quest.endNpcEntry   = fields[9].Get<uint32>();

        m_quests[quest.questId] = quest;
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance quests", count);
}

bool CampaignOfLightManager::AddQuest(CampaignQuest const& quest)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_quests[quest.questId] = quest;
    LOG_INFO("CampaignOfLight", "Added quest {} (ID: {})", quest.title, quest.questId);
    return true;
}

bool CampaignOfLightManager::StartQuest(Player* player, uint32 questId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_quests.find(questId);
    if (it == m_quests.end())
        return false;

    CampaignQuest const& quest = it->second;

    LOG_INFO("CampaignOfLight", "Player {} started quest {} (ID: {})",
        player->GetName(), quest.title, questId);

    // TODO: Actually give quest to player via player->AddQuest()
    return true;
}

bool CampaignOfLightManager::CompleteQuest(Player* player, uint32 questId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_quests.find(questId);
    if (it == m_quests.end())
        return false;

    CampaignQuest const& quest = it->second;

    LOG_INFO("CampaignOfLight", "Player {} completed quest {} (ID: {})",
        player->GetName(), quest.title, questId);

    // TODO: Give rewards
    // player->ModifyMoney(static_cast<int32>(quest.rewardGold));
    // player->RewardXp(quest.rewardXp, nullptr);
    return true;
}

CampaignQuest const* CampaignOfLightManager::GetQuest(uint32 questId) const
{
    auto it = m_quests.find(questId);
    return it != m_quests.end() ? &it->second : nullptr;
}

// ---------- Boss ----------

void CampaignOfLightManager::LoadBossesFromDb()
{
    auto result = WorldDatabase.Query(
        "SELECT entry, name, title, level, health, mana, abilities, spawn_script, "
        "float_x, float_y, float_z, map_id, respawn_time "
        "FROM campaign_bosses WHERE faction = 'alliance' OR faction = 'neutral'");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        CampaignBoss boss{};
        boss.entry       = fields[0].Get<uint32>();
        boss.name        = fields[1].Get<std::string>();
        boss.title       = fields[2].Get<std::string>();
        boss.level       = static_cast<uint8>(fields[3].Get<uint32>());
        boss.health      = fields[4].Get<uint32>();
        boss.mana        = fields[5].Get<uint32>();
        boss.abilities   = fields[6].Get<std::string>();
        boss.spawnScript = fields[7].Get<std::string>();
        boss.x           = fields[8].Get<float>();
        boss.y           = fields[9].Get<float>();
        boss.z           = fields[10].Get<float>();
        boss.mapId       = fields[11].Get<uint32>();
        boss.respawnTime = fields[12].Get<uint32>();

        m_bosses[boss.entry] = boss;
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral bosses", count);
}

bool CampaignOfLightManager::SpawnBoss(uint32 bossEntry, float x, float y, float z, uint32 mapId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bosses.find(bossEntry);
    if (it == m_bosses.end())
        return false;

    CampaignBoss const& boss = it->second;
    LOG_INFO("CampaignOfLight", "Spawning boss {} (entry {}) at ({}, {}, {}) map {}",
        boss.name, boss.entry, x, y, z, mapId);

    // TODO: Actually spawn boss creature
    // Map* map = ObjectAccessor::FindMap(mapId);
    // if (map) { map->SpawnCreature(...); }
    return true;
}

bool CampaignOfLightManager::RemoveBoss(uint32 bossEntry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bosses.erase(bossEntry) > 0;
}

std::vector<CampaignBoss> CampaignOfLightManager::GetBosses() const
{
    std::vector<CampaignBoss> list;
    list.reserve(m_bosses.size());
    for (auto const& [_, boss] : m_bosses)
        list.push_back(boss);
    return list;
}

// ---------- Shop ----------

void CampaignOfLightManager::LoadShopsFromDb()
{
    auto result = WorldDatabase.Query(
        "SELECT vendor_entry, name, items, lua_script, discount_horde, discount_alliance "
        "FROM campaign_shops WHERE faction = 'alliance' OR faction = 'neutral'");

    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        CampaignShop shop{};
        shop.vendorEntry     = fields[0].Get<uint32>();
        shop.name            = fields[1].Get<std::string>();
        shop.items           = fields[2].Get<std::string>();
        shop.luaScript       = fields[3].Get<std::string>();
        shop.discountHorde   = fields[4].Get<float>();
        shop.discountAlliance = fields[5].Get<float>();

        m_shops[shop.vendorEntry] = shop;
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral shops", count);
}

bool CampaignOfLightManager::AddShop(CampaignShop const& shop)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shops[shop.vendorEntry] = shop;
    LOG_INFO("CampaignOfLight", "Added shop {} (vendor {})", shop.name, shop.vendorEntry);
    return true;
}

bool CampaignOfLightManager::ShowShop(Player* player, uint32 vendorEntry)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_shops.find(vendorEntry);
    if (it == m_shops.end())
        return false;

    CampaignShop const& shop = it->second;
    LOG_INFO("CampaignOfLight", "Showing shop {} to player {}", shop.name, player->GetName());

    // TODO: Actually open vendor window
    // player->SendListInventory(vendorEntry);
    return true;
}

// ============================================================================
// Command handlers (AC pattern: ChatHandler*, char const* args)
// ============================================================================

bool HandleStartCampaignCommand(ChatHandler* handler, char const* /*args*/)
{
    Player* player = handler->GetPlayer();
    if (player)
        LOG_INFO("CampaignOfLight", "Player {} started Campaign of Light", player->GetName());

    handler->SendSysMessage("Campaign of Light started.");
    return true;
}

bool HandleSpawnNpcCommand(ChatHandler* handler, char const* args)
{
    if (!args || !args[0])
    {
        handler->SendSysMessage("Usage: .cnp spawn <entry>");
        return true;
    }

    uint32 entry = static_cast<uint32>(std::stoul(args));
    auto const* npc = CampaignOfLightManager::Instance().GetNpc(entry);
    if (npc)
    {
        handler->PSendSysMessage("NPC found: {} (entry {}, level {}, map {})",
            npc->name, npc->entry, npc->level, npc->mapId);
    }
    else
    {
        handler->SendSysMessage("NPC not found in database.");
    }
    return true;
}

bool HandleSpawnBossCommand(ChatHandler* handler, char const* /*args*/)
{
    handler->SendSysMessage("Spawn boss command (placeholder).");
    return true;
}

bool HandleShowDialogueCommand(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetPlayer();
    if (!player)
    {
        handler->SendSysMessage("Player not found.");
        return true;
    }

    if (!args || !args[0])
    {
        handler->SendSysMessage("Usage: .cnp dialogue <npc_entry> <node_id>");
        return true;
    }

    std::string_view sv(args);
    size_t space = sv.find(' ');
    if (space == std::string_view::npos)
    {
        handler->SendSysMessage("Usage: .cnp dialogue <npc_entry> <node_id>");
        return true;
    }

    uint32 npcEntry = static_cast<uint32>(std::stoul(std::string(sv.substr(0, space))));
    uint32 nodeId   = static_cast<uint32>(std::stoul(std::string(sv.substr(space + 1))));

    if (CampaignOfLightManager::Instance().ShowDialogue(player, npcEntry, nodeId))
        LOG_INFO("CampaignOfLight", "Dialogue shown for player {}", player->GetName());
    else
        handler->SendSysMessage("Dialogue not found.");

    return true;
}

bool HandleListNpcsCommand(ChatHandler* handler, char const* /*args*/)
{
    auto npcs = CampaignOfLightManager::Instance().GetNpcs();
    if (npcs.empty())
    {
        handler->SendSysMessage("Alliance NPCs not found.");
        return true;
    }

    handler->PSendSysMessage("Alliance NPCs (total: {}):", static_cast<uint32>(npcs.size()));
    for (auto const& npc : npcs)
        handler->PSendSysMessage("- {} (entry: {}, level: {}, map: {})",
            npc.name, npc.entry, npc.level, npc.mapId);

    return true;
}

bool HandleListBossesCommand(ChatHandler* handler, char const* /*args*/)
{
    auto bosses = CampaignOfLightManager::Instance().GetBosses();
    if (bosses.empty())
    {
        handler->SendSysMessage("Alliance bosses not found.");
        return true;
    }

    handler->PSendSysMessage("Alliance bosses (total: {}):", static_cast<uint32>(bosses.size()));
    for (auto const& boss : bosses)
        handler->PSendSysMessage("- {} {} (entry: {}, level: {}, HP: {})",
            boss.title.empty() ? "" : boss.title,
            boss.name, boss.entry, boss.level, boss.health);

    return true;
}

bool HandleShowShopCommand(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetPlayer();
    if (!player)
    {
        handler->SendSysMessage("Player not found.");
        return true;
    }

    if (!args || !args[0])
    {
        handler->SendSysMessage("Usage: .cnp shop <vendor_entry>");
        return true;
    }

    uint32 vendorEntry = static_cast<uint32>(std::stoul(args));
    if (CampaignOfLightManager::Instance().ShowShop(player, vendorEntry))
        LOG_INFO("CampaignOfLight", "Shop shown for player {}", player->GetName());
    else
        handler->SendSysMessage("Shop not found.");

    return true;
}

bool HandleCampaignInfoCommand(ChatHandler* handler, char const* /*args*/)
{
    auto const& mgr = CampaignOfLightManager::Instance();
    auto npcs  = mgr.GetNpcs();
    auto bosses = mgr.GetBosses();

    handler->PSendSysMessage("Campaign of Light: NPCs={}, Bosses={}",
        static_cast<uint32>(npcs.size()), static_cast<uint32>(bosses.size()));

    return true;
}

// ============================================================================
// Command registration
// ============================================================================

void RegisterCampaignOfLightCommands()
{
    LOG_INFO("CampaignOfLight", "Registering Campaign of Light commands");
}

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeCampaignOfLightModule()
    {
        CampaignOfLightManager::Instance().Initialize();
        LOG_INFO("CampaignOfLight", "CampaignOfLight module initialized");
    }

    __declspec(dllexport) void __stdcall FinalizeCampaignOfLightModule()
    {
        LOG_INFO("CampaignOfLight", "CampaignOfLight module finalized");
    }
}

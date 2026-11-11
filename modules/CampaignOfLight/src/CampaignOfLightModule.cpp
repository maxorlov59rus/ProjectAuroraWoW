/*
 * CampaignOfLight module - implementation
 *
 * Alliance campaign content: NPCs, dialogues, quests, bosses, shops.
 * Counterpart to CampaignOfDarkness (Horde campaign).
 *
 * This module registers two AzerothCore scripts:
 *   - CampaignOfLightCommandScript : CommandScript  (chat commands)
 *   - CampaignOfLightPlayerScript  : PlayerScript   (login / level-up hooks)
 *
 * Data is loaded lazily from the database on the first command or hook call.
 */

#include "CampaignOfLightModule.h"

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
#include <DatabaseEnv.h>
#include <Map.h>
#include <Creature.h>

#include <algorithm>
#include <cctype>

// ============================================================================
// CampaignOfLightManager - implementation
// ============================================================================

bool CampaignOfLightManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.CampaignOfLight.Enabled", "CampaignOfLight");

    if (!m_enabled)
    {
        LOG_INFO("CampaignOfLight", "Campaign of Light disabled by Aurora.CampaignOfLight.Enabled");
        return true;
    }

    m_faction = Aurora::Settings::Get<std::string>("Aurora.CampaignOfLight.Faction", "alliance", "CampaignOfLight");
    m_includeNeutral = Aurora::Settings::Enabled("Aurora.CampaignOfLight.IncludeNeutral", "CampaignOfLight", true);
    m_requireAlliance = Aurora::Settings::Enabled("Aurora.CampaignOfLight.RequireAlliance", "CampaignOfLight", true);
    m_minLevelToUnlock = Aurora::Settings::Bounded<uint32>(
        "Aurora.CampaignOfLight.MinLevelToUnlock", 10, 1, 80, "CampaignOfLight");
    m_bossRespawnMultiplier = Aurora::Settings::Bounded<float>(
        "Aurora.CampaignOfLight.BossRespawnMultiplier", 1.0f, 0.0f, 100.0f, "CampaignOfLight");

    // PreloadOnStartup is the one option that changes *when* work happens: with
    // it off the campaign tables are read on the first command instead.
    if (Aurora::Settings::Enabled("Aurora.CampaignOfLight.PreloadOnStartup", "CampaignOfLight", false))
        EnsureLoaded();

    LOG_INFO("CampaignOfLight",
        "Campaign of Light module initialized (faction '{}', neutral NPCs {}: {}, from level {}, boss respawn x{})",
        m_faction, m_includeNeutral ? "included" : "excluded",
        m_requireAlliance ? "alliance only" : "any faction", m_minLevelToUnlock, m_bossRespawnMultiplier);
    return true;
}

std::string CampaignOfLightManager::FactionCondition(std::string const& column) const
{
    // The value comes from the config, not from user input; it is still quoted
    // and restricted to the identifiers the campaign_* tables use.
    auto isSafeName = [](std::string const& text)
    {
        return !text.empty() && std::all_of(text.begin(), text.end(), [](char c)
        {
            return std::isalnum(static_cast<unsigned char>(c)) != 0;
        });
    };

    if (!isSafeName(m_faction))
        return "0";

    std::string condition = column + " = '" + m_faction + "'";
    if (m_includeNeutral)
        condition += " OR " + column + " = 'neutral'";

    return "(" + condition + ")";
}

uint32 CampaignOfLightManager::RespawnTimeFor(CampaignBoss const& boss) const
{
    return static_cast<uint32>(static_cast<double>(boss.respawnTime) * m_bossRespawnMultiplier);
}

void CampaignOfLightManager::EnsureLoaded()
{
    std::call_once(m_loadFlag, [this]() { LoadAllFromDb(); });
}

void CampaignOfLightManager::LoadAllFromDb()
{
    LOG_INFO("CampaignOfLight", "Loading Campaign of Light data from database...");

    LoadNpcsFromDb();
    LoadQuestsFromDb();
    LoadBossesFromDb();
    LoadShopsFromDb();

    m_loaded = true;
    LOG_INFO("CampaignOfLight", "Campaign of Light loaded ({} NPCs, {} quests, {} bosses, {} shops)",
        m_store.NpcCount(), m_store.QuestCount(),
        m_store.BossCount(), m_store.ShopCount());
}

// ---------- NPC ----------

void CampaignOfLightManager::LoadNpcsFromDb()
{
    std::string sql =
        "SELECT entry, name, title, level, health, float_x, float_y, float_z, "
        "map_id, dialogues_script, is_boss, is_vendor "
        "FROM campaign_npcs WHERE " + FactionCondition("faction");

    auto result = WorldDatabase.Query(sql);

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

        m_store.PutNpc(npc);
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral NPCs", count);
}

bool CampaignOfLightManager::AddNpc(CampaignNpc const& npc)
{
    m_store.PutNpc(npc);
    LOG_INFO("CampaignOfLight", "Added NPC {} (entry {})", npc.name, npc.entry);
    return true;
}

bool CampaignOfLightManager::RemoveNpc(uint32 entry)
{
    return m_store.RemoveNpc(entry);
}

std::optional<CampaignNpc> CampaignOfLightManager::GetNpc(uint32 entry) const
{
    return m_store.GetNpc(entry);
}

std::vector<CampaignNpc> CampaignOfLightManager::GetNpcs() const
{
    return m_store.GetNpcs();
}

// ---------- Dialogue ----------

bool CampaignOfLightManager::AddDialogue(uint32 npcEntry, DialogueNode const& node)
{
    m_store.PutDialogue(npcEntry, node);
    LOG_INFO("CampaignOfLight", "Added dialogue node {} for NPC {}", node.id, npcEntry);
    return true;
}

bool CampaignOfLightManager::ShowDialogue(Player* player, uint32 npcEntry, uint32 nodeId)
{
    auto Node = m_store.GetDialogue(npcEntry, nodeId);
    if (!Node)
        return false;

    DialogueNode const& node = *Node;
    LOG_INFO("CampaignOfLight", "Showing dialogue {} for player {} (NPC {})",
        nodeId, player->GetName(), npcEntry);

    // TODO: Send dialogue packet to client
    // player->SendDirectMessage(node.text);
    return true;
}

// ---------- Quest ----------

void CampaignOfLightManager::LoadQuestsFromDb()
{
    std::string sql =
        "SELECT quest_id, title, description, level, reward_gold, reward_xp, "
        "reward_items, lua_script, start_npc_entry, end_npc_entry "
        "FROM campaign_quests WHERE " + FactionCondition("faction");

    auto result = WorldDatabase.Query(sql);

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

        m_store.PutQuest(quest);
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance quests", count);
}

bool CampaignOfLightManager::AddQuest(CampaignQuest const& quest)
{
    m_store.PutQuest(quest);
    LOG_INFO("CampaignOfLight", "Added quest {} (ID: {})", quest.title, quest.questId);
    return true;
}

bool CampaignOfLightManager::StartQuest(Player* player, uint32 questId)
{
    if (!player)
        return false;

    // RequireAlliance/MinLevelToUnlock gate the campaign here so that every
    // caller (command or script) is checked the same way.
    if (m_requireAlliance && player->GetTeamId() != TEAM_ALLIANCE)
    {
        ChatHandler(player->GetSession()).SendSysMessage("The Campaign of Light is an Alliance campaign.");
        return false;
    }

    if (player->GetLevel() < m_minLevelToUnlock)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("The Campaign of Light unlocks at level {}.",
            m_minLevelToUnlock);
        return false;
    }

    auto Found = m_store.GetQuest(questId);
    if (!Found)
        return false;

    CampaignQuest const& quest = *Found;

    LOG_INFO("CampaignOfLight", "Player {} started quest {} (ID: {})",
        player->GetName(), quest.title, questId);

    // TODO: Actually give quest to player via player->AddQuest()
    return true;
}

bool CampaignOfLightManager::CompleteQuest(Player* player, uint32 questId)
{
    auto Found = m_store.GetQuest(questId);
    if (!Found)
        return false;

    CampaignQuest const& quest = *Found;

    LOG_INFO("CampaignOfLight", "Player {} completed quest {} (ID: {})",
        player->GetName(), quest.title, questId);

    // TODO: Give rewards
    // player->ModifyMoney(static_cast<int32>(quest.rewardGold));
    // player->RewardXp(quest.rewardXp, nullptr);
    return true;
}

std::optional<CampaignQuest> CampaignOfLightManager::GetQuest(uint32 questId) const
{
    return m_store.GetQuest(questId);
}

// ---------- Boss ----------

void CampaignOfLightManager::LoadBossesFromDb()
{
    std::string sql =
        "SELECT entry, name, title, level, health, mana, abilities, spawn_script, "
        "float_x, float_y, float_z, map_id, respawn_time "
        "FROM campaign_bosses WHERE " + FactionCondition("faction");

    auto result = WorldDatabase.Query(sql);

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

        m_store.PutBoss(boss);
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral bosses", count);
}

bool CampaignOfLightManager::SpawnBoss(uint32 bossEntry, float x, float y, float z, uint32 mapId)
{
    auto Found = m_store.GetBoss(bossEntry);
    if (!Found)
        return false;

    CampaignBoss const& boss = *Found;
    // BossRespawnMultiplier is what the respawn timer will actually be, so the
    // log line reports the scaled value rather than the raw database one.
    LOG_INFO("CampaignOfLight", "Spawning boss {} (entry {}) at ({}, {}, {}) map {}, respawn {} s",
        boss.name, boss.entry, x, y, z, mapId, RespawnTimeFor(boss));

    // TODO: Actually spawn boss creature
    // Map* map = ObjectAccessor::FindMap(mapId);
    // if (map) { map->SpawnCreature(...); }
    return true;
}

bool CampaignOfLightManager::RemoveBoss(uint32 bossEntry)
{
    return m_store.RemoveBoss(bossEntry);
}

std::vector<CampaignBoss> CampaignOfLightManager::GetBosses() const
{
    return m_store.GetBosses();
}

// ---------- Shop ----------

void CampaignOfLightManager::LoadShopsFromDb()
{
    std::string sql =
        "SELECT vendor_entry, name, items, lua_script, discount_horde, discount_alliance "
        "FROM campaign_shops WHERE " + FactionCondition("faction");

    auto result = WorldDatabase.Query(sql);

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

        m_store.PutShop(shop);
        ++count;
    } while (result->NextRow());

    LOG_INFO("CampaignOfLight", "Loaded {} Alliance/neutral shops", count);
}

bool CampaignOfLightManager::AddShop(CampaignShop const& shop)
{
    m_store.PutShop(shop);
    LOG_INFO("CampaignOfLight", "Added shop {} (vendor {})", shop.name, shop.vendorEntry);
    return true;
}

bool CampaignOfLightManager::ShowShop(Player* player, uint32 vendorEntry)
{
    auto Found = m_store.GetShop(vendorEntry);
    if (!Found)
        return false;

    CampaignShop const& shop = *Found;
    LOG_INFO("CampaignOfLight", "Showing shop {} to player {}", shop.name, player->GetName());

    // TODO: Actually open vendor window
    // player->SendListInventory(vendorEntry);
    return true;
}

// ============================================================================
// CommandScript - real command registration
// ============================================================================

class CampaignOfLightCommandScript : public CommandScript
{
public:
    CampaignOfLightCommandScript() : CommandScript("CampaignOfLightCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable cnpTable =
        {
            { "start",      HandleStartCampaignCommand,    rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "spawn",      HandleSpawnNpcCommand,          rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "spawnboss",  HandleSpawnBossCommand,         rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "dialogue",   HandleShowDialogueCommand,      rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "listnpcs",   HandleListNpcsCommand,          rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "listbosses", HandleListBossesCommand,        rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "shop",       HandleShowShopCommand,          rbac::RBAC_PERM_COMMAND_GM, Console::No  },
            { "info",       HandleCampaignInfoCommand,      rbac::RBAC_PERM_COMMAND_GM, Console::No  },
        };

        static ChatCommandTable commandTable =
        {
            { "cnp", cnpTable }
        };

        return commandTable;
    }

private:
    // ---------- .cnp start ----------
    static bool HandleStartCampaignCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfLightManager& mgr = CampaignOfLightManager::Instance();
        Player* player = handler->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage("The campaign commands need an in-game character.");
            return true;
        }

        if (mgr.RequireAlliance() && player->GetTeamId() != TEAM_ALLIANCE)
        {
            handler->SendSysMessage("The Campaign of Light is an Alliance campaign.");
            return true;
        }

        if (player->GetLevel() < mgr.MinLevelToUnlock())
        {
            handler->PSendSysMessage("The Campaign of Light unlocks at level {} (you are {}).",
                mgr.MinLevelToUnlock(), player->GetLevel());
            return true;
        }

        LOG_INFO("CampaignOfLight", "Player {} started Campaign of Light", player->GetName());
        handler->SendSysMessage("Campaign of Light started.");
        return true;
    }

    // ---------- .cnp spawn <entry> ----------
    static bool HandleSpawnNpcCommand(ChatHandler* handler, uint32 entry)
    {
        CampaignOfLightManager::Instance().EnsureLoaded();

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

    // ---------- .cnp spawnboss <entry> ----------
    static bool HandleSpawnBossCommand(ChatHandler* handler, uint32 entry)
    {
        CampaignOfLightManager::Instance().EnsureLoaded();

        auto bosses = CampaignOfLightManager::Instance().GetBosses();
        auto it = std::find_if(bosses.begin(), bosses.end(),
            [entry](CampaignBoss const& b) { return b.entry == entry; });

        if (it != bosses.end())
        {
            handler->PSendSysMessage("Boss found: {} {} (entry {}, level {}, HP: {})",
                it->title, it->name, it->entry, it->level, it->health);
        }
        else
        {
            handler->SendSysMessage("Boss not found in database.");
        }
        return true;
    }

    // ---------- .cnp dialogue <npc_entry> <node_id> ----------
    static bool HandleShowDialogueCommand(ChatHandler* handler, uint32 npcEntry, uint32 nodeId)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found.");
            return true;
        }

        CampaignOfLightManager::Instance().EnsureLoaded();

        if (CampaignOfLightManager::Instance().ShowDialogue(player, npcEntry, nodeId))
            LOG_INFO("CampaignOfLight", "Dialogue shown for player {}", player->GetName());
        else
            handler->SendSysMessage("Dialogue not found.");

        return true;
    }

    // ---------- .cnp listnpcs ----------
    static bool HandleListNpcsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfLightManager::Instance().EnsureLoaded();

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

    // ---------- .cnp listbosses ----------
    static bool HandleListBossesCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfLightManager::Instance().EnsureLoaded();

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

    // ---------- .cnp shop <vendor_entry> ----------
    static bool HandleShowShopCommand(ChatHandler* handler, uint32 vendorEntry)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found.");
            return true;
        }

        CampaignOfLightManager::Instance().EnsureLoaded();

        if (CampaignOfLightManager::Instance().ShowShop(player, vendorEntry))
            LOG_INFO("CampaignOfLight", "Shop shown for player {}", player->GetName());
        else
            handler->SendSysMessage("Shop not found.");

        return true;
    }

    // ---------- .cnp info ----------
    static bool HandleCampaignInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfLightManager::Instance().EnsureLoaded();

        auto const& mgr = CampaignOfLightManager::Instance();
        auto npcs  = mgr.GetNpcs();
        auto bosses = mgr.GetBosses();

        handler->PSendSysMessage("Campaign of Light: NPCs={}, Bosses={}",
            static_cast<uint32>(npcs.size()), static_cast<uint32>(bosses.size()));
        handler->PSendSysMessage("Access: {} from level {}, faction '{}' (neutral {}), boss respawn x{}",
            mgr.RequireAlliance() ? "Alliance only" : "any faction", mgr.MinLevelToUnlock(),
            mgr.Faction(), mgr.IncludeNeutral() ? "included" : "excluded", mgr.BossRespawnMultiplier());

        return true;
    }
};

// ============================================================================
// PlayerScript - login / level-up hooks
// ============================================================================

class CampaignOfLightPlayerScript : public PlayerScript
{
public:
    CampaignOfLightPlayerScript() : PlayerScript("CampaignOfLightPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;

        CampaignOfLightManager::Instance().EnsureLoaded();

        LOG_INFO("CampaignOfLight", "Player {} (guid {}) logged in - Campaign of Light available",
            player->GetName(), player->GetGUID().GetRawValue());

        // TODO: Check if player has already started this campaign
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        if (!player)
            return;

        LOG_INFO("CampaignOfLight", "Player {} leveled up to {}", player->GetName(), player->GetLevel());
    }
};

// ============================================================================
// Static instances - auto-register via constructors
// ============================================================================

namespace
{
    // Options live in configs/modules/, which the core loads after
    // worldserver.conf, so Initialize() runs here rather than at static-init
    // time (where the values would not be available yet).
    class CampaignOfLightWorldScript : public WorldScript
    {
    public:
        CampaignOfLightWorldScript() : WorldScript("CampaignOfLightWorldScript") { }

        void OnStartup() override
        {
            CampaignOfLightManager::Instance().Initialize();
        }
    };




} // namespace
// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddCampaignOfLightScripts()
{
    new CampaignOfLightCommandScript();
    new CampaignOfLightPlayerScript();
    new CampaignOfLightWorldScript();
}

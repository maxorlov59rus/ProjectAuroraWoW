/*
 * CampaignOfDarkness module - implementation
 *
 * Horde campaign content: NPCs, dialogues, quests, bosses, shops.
 * Counterpart to CampaignOfLight (Alliance campaign).
 *
 * This module registers two AzerothCore scripts:
 *   - CampaignOfDarknessCommandScript : CommandScript (chat commands)
 *   - CampaignOfDarknessPlayerScript  : PlayerScript  (login / level-up hooks)
 *
 * The manager keeps the campaign data in memory; it is filled through the
 * Add/Load API (typically from a spawn list file at startup).
 */

#include "CampaignOfDarknessModule.h"

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

#include <algorithm>
#include <fstream>
#include <sstream>

// ============================================================================
// CampaignOfDarknessManager - implementation
// ============================================================================

bool CampaignOfDarknessManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.CampaignOfDarkness.Enabled", "CampaignOfDarkness");

    if (!m_enabled)
    {
        LOG_INFO("CampaignOfDarkness", "Campaign of Darkness disabled by Aurora.CampaignOfDarkness.Enabled");
        return true;
    }

    m_requireHorde = Aurora::Settings::Enabled("Aurora.CampaignOfDarkness.RequireHorde", "CampaignOfDarkness", true);
    m_minLevelToUnlock = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.CampaignOfDarkness.MinLevelToUnlock", 10, 1, 80, "CampaignOfDarkness");
    m_defaultFaction = Aurora::Settings::Get<uint32_t>(
        "Aurora.CampaignOfDarkness.DefaultFaction", 2142, "CampaignOfDarkness");
    m_rewardGoldMultiplier = Aurora::Settings::Bounded<float>(
        "Aurora.CampaignOfDarkness.RewardGoldMultiplier", 1.0f, 0.0f, 100.0f, "CampaignOfDarkness");

    // An empty path is the normal case: the campaign is then filled through
    // the Add*/Load* API instead of a file shipped with the server.
    std::string spawnList = Aurora::Settings::Get<std::string>(
        "Aurora.CampaignOfDarkness.SpawnListFile", "", "CampaignOfDarkness");

    if (!spawnList.empty())
    {
        if (!LoadSpawnList(spawnList))
            LOG_ERROR("CampaignOfDarkness", "Spawn list '{}' could not be loaded", spawnList);
    }

    LOG_INFO("CampaignOfDarkness",
        "Campaign of Darkness module initialized ({} from level {}, faction {}, gold reward x{})",
        m_requireHorde ? "horde only" : "any faction", m_minLevelToUnlock, m_defaultFaction,
        m_rewardGoldMultiplier);
    return true;
}

uint32_t CampaignOfDarknessManager::RewardGoldFor(const CampaignQuest& quest) const
{
    return static_cast<uint32_t>(static_cast<double>(quest.rewardGold) * m_rewardGoldMultiplier);
}

void CampaignOfDarknessManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;

    LOG_INFO("CampaignOfDarkness", "Player {} logged in - {} campaign NPCs, {} quests available",
        p->GetName(), static_cast<uint32>(m_store.NpcCount()), static_cast<uint32>(m_store.QuestCount()));
}

void CampaignOfDarknessManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;

    LOG_INFO("CampaignOfDarkness", "Player {} leveled up to {}", p->GetName(), p->GetLevel());
}

// ---------- NPC ----------

bool CampaignOfDarknessManager::AddNpc(const CampaignNpc& npc)
{
    CampaignNpc stored = npc;
    // Spawn lists and scripts routinely leave the faction empty; the campaign
    // NPCs then have to share one configured faction instead of being hostile
    // to both factions by accident.
    if (stored.faction == 0)
        stored.faction = m_defaultFaction;

    m_store.PutNpc(stored);
    LOG_INFO("CampaignOfDarkness", "Added NPC {} (entry {})", stored.name, stored.entry);
    return true;
}

bool CampaignOfDarknessManager::RemoveNpc(uint32_t entry)
{
    return m_store.RemoveNpc(entry);
}

std::vector<CampaignNpc> CampaignOfDarknessManager::GetNpcs() const
{
    return m_store.GetNpcs();
}

// ---------- Dialogue ----------

bool CampaignOfDarknessManager::AddDialogue(uint32_t npcEntry, const DialogueNode& node)
{
    m_store.PutDialogue(npcEntry, node);
    return true;
}

bool CampaignOfDarknessManager::ShowDialogue(Player* p, uint32_t npcEntry, uint32_t nodeId)
{
    if (!p)
        return false;

    auto node = m_store.GetDialogue(npcEntry, nodeId);
    if (!node)
        return false;

    // TODO: send the dialogue packet to the client (see CampaignOfLight).
    LOG_INFO("CampaignOfDarkness", "Dialogue {} shown to player {} (npc {}): {}",
        nodeId, p->GetName(), npcEntry, node->text);
    return true;
}

// ---------- Quest ----------

bool CampaignOfDarknessManager::AddQuest(const CampaignQuest& quest)
{
    m_store.PutQuest(quest);
    LOG_INFO("CampaignOfDarkness", "Added quest {} (ID: {})", quest.title, quest.questId);
    return true;
}

bool CampaignOfDarknessManager::StartQuest(Player* p, uint32_t questId)
{
    if (!p)
        return false;

    // The campaign is a horde chain by configuration; the check lives here rather
    // than in the command handler so every entry point is covered.
    if (m_requireHorde && p->GetTeamId() != TEAM_HORDE)
    {
        ChatHandler(p->GetSession()).SendSysMessage("The Campaign of Darkness is a Horde campaign.");
        return false;
    }

    if (p->GetLevel() < m_minLevelToUnlock)
    {
        ChatHandler(p->GetSession()).PSendSysMessage("The Campaign of Darkness unlocks at level {}.",
            m_minLevelToUnlock);
        return false;
    }

    auto quest = m_store.GetQuest(questId);
    if (!quest)
        return false;

    LOG_INFO("CampaignOfDarkness", "Player {} started quest {} (ID: {})", p->GetName(), quest->title, questId);

    // TODO: give the quest to the player via Player::AddQuest() once the
    // quest templates are imported into the world database.
    return true;
}

bool CampaignOfDarknessManager::CompleteQuest(Player* p, uint32_t questId)
{
    if (!p)
        return false;

    uint32 rewardGold = 0;
    std::string title;

    auto quest = m_store.GetQuest(questId);
    if (!quest)
        return false;

    title = quest->title;
    rewardGold = RewardGoldFor(*quest);

    LOG_INFO("CampaignOfDarkness", "Player {} completed quest {} (ID: {})", p->GetName(), quest->title, questId);

    // RewardGoldMultiplier is only visible to players if the payout it produced
    // is what they are told about.
    ChatHandler(p->GetSession()).PSendSysMessage("Quest '{}' completed, reward: {} gold.", title, rewardGold);

    // TODO: grant the reward (gold/xp) once quests live in the world database.
    return true;
}

// ---------- Boss ----------

bool CampaignOfDarknessManager::SpawnBoss(uint32_t bossEntry)
{
    auto boss = m_store.GetBoss(bossEntry);
    if (!boss)
        return false;

    LOG_INFO("CampaignOfDarkness", "Spawning boss {} (entry {}) at ({}, {}, {}) map {}",
        boss->name, boss->entry, boss->spawnLocationX, boss->spawnLocationY, boss->spawnLocationZ, boss->mapId);

    // TODO: actually create the creature through the map spawn helper.
    return true;
}

bool CampaignOfDarknessManager::RemoveBoss(uint32_t bossEntry)
{
    return m_store.RemoveBoss(bossEntry);
}

std::vector<CampaignBoss> CampaignOfDarknessManager::GetBosses() const
{
    return m_store.GetBosses();
}

// ---------- Shop ----------

bool CampaignOfDarknessManager::AddShop(const CampaignShop& shop)
{
    m_store.PutShop(shop);
    LOG_INFO("CampaignOfDarkness", "Added shop {} (vendor {})", shop.name, shop.vendorEntry);
    return true;
}

bool CampaignOfDarknessManager::ShowShop(Player* p, uint32_t vendorEntry)
{
    if (!p)
        return false;

    auto shop = m_store.GetShop(vendorEntry);
    if (!shop)
        return false;

    // TODO: open the vendor list on the client once the vendor creature exists.
    // Player has no printf-style accessor, so the message goes through a
    // temporary ChatHandler bound to the player's session.
    ChatHandler(p->GetSession()).PSendSysMessage("Campaign shop '{}' (vendor entry {}, {} items)",
        shop->name, shop->vendorEntry, static_cast<uint32>(shop->items.size()));
    return true;
}

// ---------- Spawn list ----------
//
// Line format (whitespace separated, '#' starts a comment):
//   npc   <entry> <name> <level> <map> <x> <y> <z>
//   boss  <entry> <name> <level> <map> <x> <y> <z>
// Anything else is skipped so that partially written lists stay loadable.

bool CampaignOfDarknessManager::LoadSpawnList(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        LOG_ERROR("CampaignOfDarkness", "Cannot open spawn list '{}'", filename);
        return false;
    }

    uint32 npcCount = 0;
    uint32 bossCount = 0;
    std::string line;

    // The list is parsed without holding the lock and merged in one step,
    // because AddNpc()/AddShop() take the same (non-recursive) mutex.
    std::vector<CampaignNpc> npcs;
    std::vector<CampaignBoss> bosses;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream stream(line);
        std::string kind, name;
        uint32 entry = 0, level = 0, mapId = 0;
        float x = 0.0f, y = 0.0f, z = 0.0f;

        if (!(stream >> kind >> entry >> name >> level >> mapId >> x >> y >> z))
            continue;

        if (kind == "npc")
        {
            CampaignNpc npc{};
            npc.entry = entry;
            npc.name = name;
            npc.level = level;
            npc.mapId = mapId;
            npc.x = x;
            npc.y = y;
            npc.z = z;
            npcs.push_back(npc);
            ++npcCount;
        }
        else if (kind == "boss")
        {
            CampaignBoss boss{};
            boss.entry = entry;
            boss.name = name;
            boss.level = level;
            boss.mapId = mapId;
            boss.spawnLocationX = static_cast<uint32_t>(x);
            boss.spawnLocationY = static_cast<uint32_t>(y);
            boss.spawnLocationZ = static_cast<uint32_t>(z);
            bosses.push_back(boss);
            ++bossCount;
        }
    }

    for (auto const& npc : npcs)
        m_store.PutNpc(npc);
    for (auto const& boss : bosses)
        m_store.PutBoss(boss);

    LOG_INFO("CampaignOfDarkness", "Loaded spawn list '{}' ({} NPCs, {} bosses)",
        filename, npcCount, bossCount);
    return true;
}

bool CampaignOfDarknessManager::SaveSpawnList(const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        LOG_ERROR("CampaignOfDarkness", "Cannot write spawn list '{}'", filename);
        return false;
    }

    file << "# Campaign of Darkness spawn list\n";
    for (auto const& npc : m_store.GetNpcs())
        file << "npc " << npc.entry << " " << npc.name << " " << npc.level << " "
             << npc.mapId << " " << npc.x << " " << npc.y << " " << npc.z << "\n";

    for (auto const& boss : m_store.GetBosses())
        file << "boss " << boss.entry << " " << boss.name << " " << boss.level << " "
             << boss.mapId << " " << boss.spawnLocationX << " "
             << boss.spawnLocationY << " " << boss.spawnLocationZ << "\n";

    LOG_INFO("CampaignOfDarkness", "Saved spawn list '{}'", filename);
    return true;
}

// ============================================================================
// CommandScript - real command registration
// ============================================================================

class CampaignOfDarknessCommandScript : public CommandScript
{
public:
    CampaignOfDarknessCommandScript() : CommandScript("CampaignOfDarknessCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable codTable =
        {
            { "start",      HandleStartCampaignCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "spawn",      HandleSpawnNpcCommand,      rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "spawnboss",  HandleSpawnBossCommand,     rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "dialogue",   HandleShowDialogueCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "listnpcs",   HandleListNpcsCommand,      rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "listbosses", HandleListBossesCommand,    rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "shop",       HandleShowShopCommand,      rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info",       HandleCampaignInfoCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "cod", codTable }
        };

        return commandTable;
    }

private:
    // ---------- .cod start ----------
    static bool HandleStartCampaignCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfDarknessManager& mgr = CampaignOfDarknessManager::Instance();
        Player* player = handler->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage("The campaign commands need an in-game character.");
            return true;
        }

        if (mgr.RequireHorde() && player->GetTeamId() != TEAM_HORDE)
        {
            handler->SendSysMessage("The Campaign of Darkness is a Horde campaign.");
            return true;
        }

        if (player->GetLevel() < mgr.MinLevelToUnlock())
        {
            handler->PSendSysMessage("The Campaign of Darkness unlocks at level {} (you are {}).",
                mgr.MinLevelToUnlock(), player->GetLevel());
            return true;
        }

        LOG_INFO("CampaignOfDarkness", "Player {} started Campaign of Darkness", player->GetName());
        handler->SendSysMessage("Campaign of Darkness started.");
        return true;
    }

    // ---------- .cod spawn <entry> ----------
    static bool HandleSpawnNpcCommand(ChatHandler* handler, uint32 entry)
    {
        auto npcs = CampaignOfDarknessManager::Instance().GetNpcs();
        auto it = std::find_if(npcs.begin(), npcs.end(),
            [entry](CampaignNpc const& npc) { return npc.entry == entry; });

        if (it != npcs.end())
            handler->PSendSysMessage("NPC found: {} (entry {}, level {}, map {})",
                it->name, it->entry, it->level, it->mapId);
        else
            handler->PSendSysMessage("NPC {} not found.", entry);

        return true;
    }

    // ---------- .cod spawnboss <entry> ----------
    static bool HandleSpawnBossCommand(ChatHandler* handler, uint32 entry)
    {
        if (CampaignOfDarknessManager::Instance().SpawnBoss(entry))
            handler->PSendSysMessage("Boss {} spawned.", entry);
        else
            handler->PSendSysMessage("Boss {} not found.", entry);

        return true;
    }

    // ---------- .cod dialogue <npc_entry> <node_id> ----------
    static bool HandleShowDialogueCommand(ChatHandler* handler, uint32 npcEntry, uint32 nodeId)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found.");
            return true;
        }

        if (!CampaignOfDarknessManager::Instance().ShowDialogue(player, npcEntry, nodeId))
            handler->SendSysMessage("Dialogue not found.");

        return true;
    }

    // ---------- .cod listnpcs ----------
    static bool HandleListNpcsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto npcs = CampaignOfDarknessManager::Instance().GetNpcs();
        if (npcs.empty())
        {
            handler->SendSysMessage("Horde NPCs not found.");
            return true;
        }

        handler->PSendSysMessage("Horde NPCs (total: {}):", static_cast<uint32>(npcs.size()));
        for (auto const& npc : npcs)
            handler->PSendSysMessage("- {} (entry: {}, level: {}, map: {})",
                npc.name, npc.entry, npc.level, npc.mapId);

        return true;
    }

    // ---------- .cod listbosses ----------
    static bool HandleListBossesCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto bosses = CampaignOfDarknessManager::Instance().GetBosses();
        if (bosses.empty())
        {
            handler->SendSysMessage("Horde bosses not found.");
            return true;
        }

        handler->PSendSysMessage("Horde bosses (total: {}):", static_cast<uint32>(bosses.size()));
        for (auto const& boss : bosses)
            handler->PSendSysMessage("- {} (entry: {}, level: {}, HP: {})",
                boss.name, boss.entry, boss.level, boss.health);

        return true;
    }

    // ---------- .cod shop <vendor_entry> ----------
    static bool HandleShowShopCommand(ChatHandler* handler, uint32 vendorEntry)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Player not found.");
            return true;
        }

        if (!CampaignOfDarknessManager::Instance().ShowShop(player, vendorEntry))
            handler->PSendSysMessage("Shop not found.");

        return true;
    }

    // ---------- .cod info ----------
    static bool HandleCampaignInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        CampaignOfDarknessManager& mgr = CampaignOfDarknessManager::Instance();
        auto npcs = mgr.GetNpcs();
        auto bosses = mgr.GetBosses();

        handler->PSendSysMessage("Campaign of Darkness: NPCs={}, Bosses={}",
            static_cast<uint32>(npcs.size()), static_cast<uint32>(bosses.size()));
        handler->PSendSysMessage("Access: {} from level {}, NPC faction {}, gold reward x{}",
            mgr.RequireHorde() ? "Horde only" : "any faction", mgr.MinLevelToUnlock(),
            mgr.DefaultFaction(), mgr.RewardGoldMultiplier());

        return true;
    }
};

// ============================================================================
// PlayerScript - login / level-up hooks
// ============================================================================

class CampaignOfDarknessPlayerScript : public PlayerScript
{
public:
    CampaignOfDarknessPlayerScript() : PlayerScript("CampaignOfDarknessPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        CampaignOfDarknessManager::Instance().OnPlayerLogin(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        CampaignOfDarknessManager::Instance().OnPlayerLevelUp(player);
    }
};

// ============================================================================
// Static instances - auto-register via constructors
// ============================================================================

namespace
{
    // See CampaignOfLight: module options are only readable in OnStartup.
    class CampaignOfDarknessWorldScript : public WorldScript
    {
    public:
        CampaignOfDarknessWorldScript() : WorldScript("CampaignOfDarknessWorldScript") { }

        void OnStartup() override
        {
            CampaignOfDarknessManager::Instance().Initialize();
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

void AddCampaignOfDarknessScripts()
{
    new CampaignOfDarknessCommandScript();
    new CampaignOfDarknessPlayerScript();
    new CampaignOfDarknessWorldScript();
}

#pragma once

// CampaignStore - общее хранилище данных кампании.
//
// Вынесено из CampaignOfLightManager по образцу модуля wow-mod-delves:
// данные, мьютекс и загрузка живут отдельно от команд и хуков, поэтому
// реестр можно тестировать без AzerothCore.
//
// Важное отличие от прежней реализации: геттеры возвращают значение, а не
// указатель на элемент std::map. Указатель под мьютексом разблокировался в
// момент возврата и умирал при первом erase(), то есть давал гонку.

#include "Define.h"

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace Aurora::Campaign
{
    struct NpcData
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

    struct DialogueNode
    {
        uint32 id = 0;
        std::string text;
        std::vector<uint32> nextNodes;
        std::string luaScript;
        bool isEnd = false;
    };

    struct QuestData
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

    struct BossData
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

    struct ShopData
    {
        uint32 vendorEntry = 0;
        std::string name;
        std::string items;
        std::string luaScript;
        float discountHorde = 0.f;
        float discountAlliance = 0.f;
    };

    /** Потокобезопасный реестр данных кампании. */
    class Store
    {
    public:
        // --- NPC ---
        void PutNpc(NpcData const& npc);
        bool RemoveNpc(uint32 entry);
        std::optional<NpcData> GetNpc(uint32 entry) const;
        std::vector<NpcData> GetNpcs() const;
        int32 NpcCount() const;

        // --- Dialogue ---
        void PutDialogue(uint32 npcEntry, DialogueNode const& node);
        std::optional<DialogueNode> GetDialogue(uint32 npcEntry, uint32 nodeId) const;

        // --- Quest ---
        void PutQuest(QuestData const& quest);
        std::optional<QuestData> GetQuest(uint32 questId) const;
        int32 QuestCount() const;

        // --- Boss ---
        void PutBoss(BossData const& boss);
        bool RemoveBoss(uint32 entry);
        std::optional<BossData> GetBoss(uint32 entry) const;
        std::vector<BossData> GetBosses() const;
        int32 BossCount() const;

        // --- Shop ---
        void PutShop(ShopData const& shop);
        std::optional<ShopData> GetShop(uint32 vendorEntry) const;
        int32 ShopCount() const;

        /** Число NPC/квестов/боссов/магазинов одной строкой для лога загрузки. */
        std::string Summary() const;

    private:
        mutable std::mutex Mutex;

        std::map<uint32, NpcData> Npcs;
        std::map<uint32, std::map<uint32, DialogueNode>> Dialogues;
        std::map<uint32, QuestData> Quests;
        std::map<uint32, BossData> Bosses;
        std::map<uint32, ShopData> Shops;
    };
}

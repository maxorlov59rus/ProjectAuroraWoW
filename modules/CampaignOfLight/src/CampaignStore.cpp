#include "CampaignStore.h"

namespace Aurora::Campaign
{
    namespace
    {
        template <class TMap>
        std::vector<typename TMap::mapped_type> DumpValues(TMap const& Source)
        {
            std::vector<typename TMap::mapped_type> Out;
            Out.reserve(Source.size());
            for (auto const& [_, Value] : Source)
            {
                Out.push_back(Value);
            }
            return Out;
        }
    }

    // ---------------- NPC ----------------

    void Store::PutNpc(NpcData const& Npc)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        Npcs[Npc.entry] = Npc;
    }

    bool Store::RemoveNpc(uint32 Entry)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return Npcs.erase(Entry) > 0;
    }

    std::optional<NpcData> Store::GetNpc(uint32 Entry) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        auto It = Npcs.find(Entry);
        if (It == Npcs.end())
        {
            return std::nullopt;
        }
        return It->second;
    }

    std::vector<NpcData> Store::GetNpcs() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return DumpValues(Npcs);
    }

    int32 Store::NpcCount() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return static_cast<int32>(Npcs.size());
    }

    // ---------------- Диалоги ----------------

    void Store::PutDialogue(uint32 NpcEntry, DialogueNode const& Node)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        Dialogues[NpcEntry][Node.id] = Node;
    }

    std::optional<DialogueNode> Store::GetDialogue(uint32 NpcEntry, uint32 NodeId) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);

        auto Npc = Dialogues.find(NpcEntry);
        if (Npc == Dialogues.end())
        {
            return std::nullopt;
        }

        auto Node = Npc->second.find(NodeId);
        if (Node == Npc->second.end())
        {
            return std::nullopt;
        }
        return Node->second;
    }

    // ---------------- Квесты ----------------

    void Store::PutQuest(QuestData const& Quest)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        Quests[Quest.questId] = Quest;
    }

    std::optional<QuestData> Store::GetQuest(uint32 QuestId) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        auto It = Quests.find(QuestId);
        if (It == Quests.end())
        {
            return std::nullopt;
        }
        return It->second;
    }

    int32 Store::QuestCount() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return static_cast<int32>(Quests.size());
    }

    // ---------------- Боссы ----------------

    void Store::PutBoss(BossData const& Boss)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        Bosses[Boss.entry] = Boss;
    }

    bool Store::RemoveBoss(uint32 Entry)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return Bosses.erase(Entry) > 0;
    }

    std::optional<BossData> Store::GetBoss(uint32 Entry) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        auto It = Bosses.find(Entry);
        if (It == Bosses.end())
        {
            return std::nullopt;
        }
        return It->second;
    }

    std::vector<BossData> Store::GetBosses() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return DumpValues(Bosses);
    }

    int32 Store::BossCount() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return static_cast<int32>(Bosses.size());
    }

    // ---------------- Магазины ----------------

    void Store::PutShop(ShopData const& Shop)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        Shops[Shop.vendorEntry] = Shop;
    }

    std::optional<ShopData> Store::GetShop(uint32 VendorEntry) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        auto It = Shops.find(VendorEntry);
        if (It == Shops.end())
        {
            return std::nullopt;
        }
        return It->second;
    }

    int32 Store::ShopCount() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return static_cast<int32>(Shops.size());
    }

    // ---------------- Сводка ----------------

    std::string Store::Summary() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return std::to_string(Npcs.size()) + " NPCs, "
            + std::to_string(Quests.size()) + " quests, "
            + std::to_string(Bosses.size()) + " bosses, "
            + std::to_string(Shops.size()) + " shops";
    }
}

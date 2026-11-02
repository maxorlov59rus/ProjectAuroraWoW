/*
 * DreamAlchemyModule - "Dream Alchemy" for AzerothCore (WotLK)
 *
 * A potion drunk in a tavern only pays off after the player has been offline
 * for the configured hours. On the next login they get a rested buff and a
 * dream shard; ten shards combine into a potion of awakening.
 */
#pragma once

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <map>
#include <mutex>

class Player;

struct DreamState
{
    int64_t drunkAt = 0;      // wall clock when the potion was drunk
    uint32_t shards = 0;
};

class DreamAlchemyManager
{
public:
    static DreamAlchemyManager& Instance();

    bool Initialize();

    // A player drinks the dream vial while in a tavern. Returns false if they
    // are not resting.
    bool DrinkVial(Player* p);

    // Called on login: grants the rested buff/shard when the offline period
    // exceeded the configured minimum.
    bool OnPlayerLogin(Player* p);

    // Combines shards into a potion of awakening.
    bool CombinePotion(Player* p);

    bool IsEnabled() const { return m_enabled; }
    bool IsResting(Player* p) const;
    uint32_t MinOfflineSecs() const { return m_minOfflineSecs; }
    uint32_t ShardsForPotion() const { return m_shardsForPotion; }
    uint32_t RestedBuffSpell() const { return m_restedBuffSpell; }
    uint32_t ShardCount(Player* p) const;

private:
    DreamAlchemyManager() = default;
    ~DreamAlchemyManager() = default;

    bool m_enabled = true;
    uint32_t m_minOfflineSecs = 8 * 3600;
    uint32_t m_shardsForPotion = 10;
    uint32_t m_restedBuffSpell = 0;
    uint32_t m_awakeningSpell = 0;

    std::map<ObjectGuid, DreamState> m_states;
    mutable std::mutex m_mutex;
};

void AddDreamAlchemyScripts();
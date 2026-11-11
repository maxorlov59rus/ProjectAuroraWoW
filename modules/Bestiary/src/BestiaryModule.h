/*
 * BestiaryModule - "Bestiary: Chaos Taxidermy" for AzerothCore (WotLK)
 *
 * Players photograph mutated creatures in areas of "spatial ripple" and turn
 * the snapshots into monster cards. Ten cards of one type become a mountable
 * trophy. The manager keeps the collection in memory and exposes it through
 * commands; creature spawning itself is deferred until the spawn data exists.
 */
#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <mutex>
#include <vector>

class Player;

struct BestiaryCard
{
    std::string creatureType; // e.g. "wolf", "spider"
    uint32_t mutation = 0;    // visual mutation variant id
    uint32_t count = 0;
};
class BestiaryManager
{
public:
    static BestiaryManager& Instance();

    bool Initialize();

    // Registers one photograph of a mutated creature. onCardComplete is set
    // when the photograph completes a set of CardSetSize.
    bool Photograph(Player* p, std::string const& creatureType, uint32 mutation, bool& onCardComplete);

    std::vector<BestiaryCard> GetCards() const;

    bool IsEnabled() const { return m_enabled; }
    uint32_t CardSetSize() const { return m_cardSetSize; }
    uint32_t MaxTrophies() const { return m_maxTrophies; }
    bool TrophyPlaced(std::string const& creatureType) const;

private:
    BestiaryManager() = default;
    ~BestiaryManager() = default;

    bool m_enabled = true;
    uint32_t m_cardSetSize = 10;
    uint32_t m_maxTrophies = 20;

    std::map<std::string, uint32_t> m_counts;   // creatureType -> card count
    std::vector<std::string> m_trophies;        // types that earned a trophy
    mutable std::mutex m_mutex;
};
void AddBestiaryScripts();
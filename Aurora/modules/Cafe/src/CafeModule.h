/*
 * CafeModule - "Cafe of Broken Hearts" for AzerothCore (WotLK)
 *
 * A social hub where players trade their failed raid runs. A player picks a
 * raid and an inventory slot; the module tracks how often a boss died without
 * dropping an item for that slot, and after enough misses the player earns a
 * "failure essence" that can be exchanged for a random hope amulet.
 *
 * Inventory scanning is represented by a deterministic mock until the item
 * lookup can be wired to the live Player::GetItemCount API, so the tracking,
 * the thresholds and the exchange are what the commands exercise.
 */
#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <mutex>

class Player;

struct CafeEntry
{
    uint32_t misses = 0;
    uint32_t essences = 0;
};

class CafeManager
{
public:
    static CafeManager& Instance();

    bool Initialize();

    // Registers a failed-drop attempt for a raid/slot. Returns the number of
    // essences earned by this attempt.
    uint32_t RegisterMiss(Player* p, std::string const& raid, std::string const& slot);

    // Exchanges essences for an amulet; returns false if not enough essences.
    bool ExchangeAmulet(Player* p, std::string const& raid, std::string const& slot);

    bool IsEnabled() const { return m_enabled; }
    uint32_t MissThreshold() const { return m_missThreshold; }
    uint32_t AmuletCost() const { return m_amuletCost; }
    uint32_t AmuletDurationSecs() const { return m_amuletDurationSecs; }
    uint32_t EssenceCount(std::string const& raid, std::string const& slot) const;

private:
    CafeManager() = default;
    ~CafeManager() = default;

    std::string Key(std::string const& raid, std::string const& slot) const;

    bool m_enabled = true;
    uint32_t m_missThreshold = 10;
    uint32_t m_amuletCost = 3;
    uint32_t m_amuletDurationSecs = 3600;

    std::map<std::string, CafeEntry> m_entries;
    mutable std::mutex m_mutex;
};

void AddCafeScripts();
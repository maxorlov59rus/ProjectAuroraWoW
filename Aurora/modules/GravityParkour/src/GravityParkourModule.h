/*
 * GravityParkourModule - hidden gravity parkour zones for AzerothCore (WotLK)
 *
 * At midnight in Darnassus, a levitating player who jumps off the temple
 * balcony reaches a hidden "Celestial Gardens" phase. The platforms there run
 * with altered gravity and, at the end, a chest grants moth wings that only
 * work in the capitals.
 */
#pragma once

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <mutex>
#include <vector>

class Player;

class GravityParkourManager
{
public:
    static GravityParkourManager& Instance();

    bool Initialize();

    // Called on player movement; enters the hidden phase when the conditions
    // (zone, midnight, levitate) are met.
    void OnPlayerUpdate(Player* p, uint32 diff);

    // Teleports the player into the celestial phase.
    bool EnterCelestialPhase(Player* p);

    // Grants the moth-wings mount if the player finished the course.
    bool GrantWings(Player* p);

    bool IsEnabled() const { return m_enabled; }
    bool IsInPhase(Player* p) const;
    uint32_t ZoneId() const { return m_zoneId; }
    uint32_t PhaseMask() const { return m_phaseMask; }

private:
    GravityParkourManager() = default;
    ~GravityParkourManager() = default;

    bool m_enabled = true;
    uint32_t m_zoneId = 1;      // Darnassus
    uint32_t m_hour = 0;        // midnight
    uint32_t m_phaseMask = 0x1; // hidden phase bit
    bool m_requireLevitate = true;
    uint32_t m_wingsSpell = 0;

    std::vector<ObjectGuid> m_phasePlayers;
    mutable std::mutex m_mutex;
};

void AddGravityParkourScripts();
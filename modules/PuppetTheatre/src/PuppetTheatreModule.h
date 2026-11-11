/*
 * PuppetTheatreModule - "Puppet Theatre" for AzerothCore (WotLK)
 *
 * A broken doll can be turned into a marionette. While the marionette is
 * active the player issues orders through four abilities (strike, dodge,
 * taunt, charge) instead of fighting directly. Defeating the champion of
 * Stormwind unlocks a chest.
 */
#pragma once

#include "Define.h"
#include "ObjectGuid.h"
#include <map>
#include <mutex>
#include <string>

class Player;

struct PuppetState
{
    bool summoned = false;
    ObjectGuid puppetGuid;   // the marionette creature
    uint32_t strikeSpell = 0;
    uint32_t dodgeSpell = 0;
    uint32_t tauntSpell = 0;
    uint32_t chargeSpell = 0;
};
class PuppetTheatreManager
{
public:
    static PuppetTheatreManager& Instance();

    bool Initialize();

    // Awakens a marionette from a broken doll for the player.
    bool SummonPuppet(Player* p);

    // Issues one of the four orders to the marionette against the current target.
    bool UseAbility(Player* p, uint32_t slot);

    // Called on logout: despawns the marionette that belongs to the player.
    void OnPlayerLogout(Player* p);

    bool IsEnabled() const { return m_enabled; }
    bool HasPuppet(Player* p) const;

private:
    PuppetTheatreManager() = default;
    ~PuppetTheatreManager() = default;

    bool m_enabled = true;
    uint32_t m_puppetEntry = 0;
    uint32_t m_brokenDollItem = 0;
    uint32_t m_championEntry = 0;
    uint32_t m_chestItem = 0;
    uint32_t m_strikeSpell = 0;
    uint32_t m_dodgeSpell = 0;
    uint32_t m_tauntSpell = 0;
    uint32_t m_chargeSpell = 0;

    std::map<ObjectGuid, PuppetState> m_puppets;
    mutable std::mutex m_mutex;
};
void AddPuppetTheatreScripts();
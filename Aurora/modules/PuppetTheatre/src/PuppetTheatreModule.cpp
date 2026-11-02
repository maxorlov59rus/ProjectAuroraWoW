/*
 * PuppetTheatreModule - implementation
 *
 * The marionette is a summoned creature that obeys four player orders. The
 * orders are simple targeted spells cast through the player, so the module
 * stays testable before the creature/spell rows ship: with the spell ids at 0
 * the commands still validate the state machine and the limits.
 */
#include "PuppetTheatreModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <WorldScript.h>
#include <Player.h>
#include <Creature.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>
#include <Map.h>
#include <ObjectAccessor.h>

// ============================================================================
// PuppetTheatreManager
// ============================================================================

PuppetTheatreManager& PuppetTheatreManager::Instance()
{
    static PuppetTheatreManager inst;
    return inst;
}

bool PuppetTheatreManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.PuppetTheatre.Enabled", "PuppetTheatre");
    m_puppetEntry = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.PuppetEntry", 0, "PuppetTheatre");
    m_brokenDollItem = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.BrokenDollItem", 0, "PuppetTheatre");
    m_championEntry = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.ChampionEntry", 0, "PuppetTheatre");
    m_chestItem = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.ChestItem", 0, "PuppetTheatre");
    m_strikeSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.StrikeSpell", 0, "PuppetTheatre");
    m_dodgeSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.DodgeSpell", 0, "PuppetTheatre");
    m_tauntSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.TauntSpell", 0, "PuppetTheatre");
    m_chargeSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.PuppetTheatre.ChargeSpell", 0, "PuppetTheatre");

    if (!m_enabled)
    {
        LOG_INFO("PuppetTheatre", "PuppetTheatre disabled by Aurora.PuppetTheatre.Enabled");
        return true;
    }

    LOG_INFO("PuppetTheatre",
        "Puppet Theatre initialized (puppet {}, doll {}, champion {}, chest {})",
        m_puppetEntry, m_brokenDollItem, m_championEntry, m_chestItem);
    return true;
}

bool PuppetTheatreManager::HasPuppet(Player* p) const
{
    if (!p)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_puppets.find(p->GetGUID());
    return it != m_puppets.end() && it->second.summoned;
}

bool PuppetTheatreManager::SummonPuppet(Player* p)
{
    if (!p || !m_enabled)
        return false;

    // The doll is optional: with the item id at 0 the command only enforces the
    // summon/limits logic, and the item requirement is skipped.
    if (m_brokenDollItem && !p->HasItemCount(m_brokenDollItem))
    {
        LOG_INFO("PuppetTheatre", "{} tried to awaken a marionette without a broken doll", p->GetName());
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_puppets.find(p->GetGUID());
        if (it != m_puppets.end() && it->second.summoned)
            return false;
    }

    // PuppetEntry = 0 is the virtual mode: no creature template exists yet, so
    // the summon is simulated in memory and only the state machine is exercised.
    ObjectGuid puppetGuid;
    if (m_puppetEntry)
    {
        Position pos(p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetOrientation());
        if (Creature* creature = p->GetMap()->SummonCreature(m_puppetEntry, pos, nullptr, 0, p, 0, 0, true))
            puppetGuid = creature->GetGUID();
        else
            return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    PuppetState& state = m_puppets[p->GetGUID()];
    state.summoned = true;
    state.puppetGuid = puppetGuid;
    state.strikeSpell = m_strikeSpell;
    state.dodgeSpell = m_dodgeSpell;
    state.tauntSpell = m_tauntSpell;
    state.chargeSpell = m_chargeSpell;

    LOG_INFO("PuppetTheatre", "{} awakened a marionette", p->GetName());
    return true;
}

bool PuppetTheatreManager::UseAbility(Player* p, uint32_t slot)
{
    if (!p || !m_enabled)
        return false;

    if (!HasPuppet(p))
        return false;

    Unit* target = p->GetSelectedUnit();
    if (!target || !target->IsAlive())
        return false;

    uint32_t spellId = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_puppets.find(p->GetGUID());
        if (it == m_puppets.end() || !it->second.summoned)
            return false;

        switch (slot)
        {
            case 1: spellId = it->second.strikeSpell; break;
            case 2: spellId = it->second.dodgeSpell;  break;
            case 3: spellId = it->second.tauntSpell;  break;
            case 4: spellId = it->second.chargeSpell; break;
            default:
                return false;
        }
    }

    // The puppet channels the order through the player. A zero spell id means
    // the order is acknowledged but has no combat effect yet.
    if (spellId)
        p->CastSpell(target, spellId, true);

    LOG_INFO("PuppetTheatre", "{} issued puppet order slot {}", p->GetName(), slot);
    return true;
}

void PuppetTheatreManager::OnPlayerLogout(Player* p)
{
    if (!p)
        return;

    ObjectGuid puppetGuid;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_puppets.find(p->GetGUID());
        if (it == m_puppets.end())
            return;
        puppetGuid = it->second.puppetGuid;
        m_puppets.erase(it);
    }

    // The marionette leaves with its master.
    if (puppetGuid)
    {
        if (Creature* creature = ObjectAccessor::GetCreature(*p, puppetGuid))
            creature->DespawnOrUnsummon();
    }
}

// ============================================================================
// CommandScript
// ============================================================================

class PuppetTheatreCommandScript : public CommandScript
{
public:
    PuppetTheatreCommandScript() : CommandScript("PuppetTheatreCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable puppetTable =
        {
            { "summon", HandleSummonCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "strike", HandleStrikeCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "dodge",  HandleDodgeCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "taunt",  HandleTauntCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "charge", HandleChargeCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "chest",  HandleChestCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "puppet", puppetTable }
        };

        return commandTable;
    }

private:
    static bool HandleSummonCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The theatre needs an in-game character.");
            return true;
        }

        if (PuppetTheatreManager::Instance().SummonPuppet(p))
            handler->SendSysMessage("Marionette awakened - issue orders with .puppet strike/dodge/taunt/charge.");
        else
            handler->SendSysMessage("Cannot awaken the marionette (missing doll, already active, or disabled).");

        return true;
    }

    static bool HandleStrikeCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        return HandleOrder(handler, 1);
    }

    static bool HandleDodgeCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        return HandleOrder(handler, 2);
    }

    static bool HandleTauntCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        return HandleOrder(handler, 3);
    }

    static bool HandleChargeCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        return HandleOrder(handler, 4);
    }

    static bool HandleOrder(ChatHandler* handler, uint32_t slot)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The theatre needs an in-game character.");
            return true;
        }

        if (PuppetTheatreManager::Instance().UseAbility(p, slot))
            handler->PSendSysMessage("Order {} issued.", slot);
        else
            handler->SendSysMessage("Order failed (no marionette or no valid target).");

        return true;
    }

    static bool HandleChestCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The theatre needs an in-game character.");
            return true;
        }

        uint32_t const chestItem = Aurora::Settings::Get<uint32_t>(
            "Aurora.PuppetTheatre.ChestItem", 0, "PuppetTheatre");
        if (chestItem)
            p->AddItem(chestItem, 1);

        handler->SendSysMessage("The champion's chest is yours!");
        return true;
    }
};

// ============================================================================
// PlayerScript / WorldScript
// ============================================================================

class PuppetTheatrePlayerScript : public PlayerScript
{
public:
    PuppetTheatrePlayerScript() : PlayerScript("PuppetTheatrePlayerScript") { }

    void OnPlayerLogout(Player* player) override
    {
        PuppetTheatreManager::Instance().OnPlayerLogout(player);
    }
};

class PuppetTheatreWorldScript : public WorldScript
{
public:
    PuppetTheatreWorldScript() : WorldScript("PuppetTheatreWorldScript") { }

    void OnStartup() override
    {
        PuppetTheatreManager::Instance().Initialize();
    }
};

void AddPuppetTheatreScripts()
{
    new PuppetTheatreCommandScript();
    new PuppetTheatrePlayerScript();
    new PuppetTheatreWorldScript();
}
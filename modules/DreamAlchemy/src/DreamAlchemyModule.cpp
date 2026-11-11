/*
 * DreamAlchemyModule - implementation
 *
 * The "healthy play" loop: rest in a tavern, log out, come back after enough
 * hours and collect the rested buff plus a dream shard. Shards combine into a
 * potion of awakening. The buffs are spell ids from the config, so the module
 * stays testable before the spell rows ship.
 */
#include "DreamAlchemyModule.h"

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
#include <GameTime.h>

// ============================================================================
// DreamAlchemyManager
// ============================================================================

DreamAlchemyManager& DreamAlchemyManager::Instance()
{
    static DreamAlchemyManager inst;
    return inst;
}

bool DreamAlchemyManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.DreamAlchemy.Enabled", "DreamAlchemy");
    m_minOfflineSecs = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.DreamAlchemy.MinOfflineSecs", 8 * 3600, 0, 7 * 86400, "DreamAlchemy");
    m_shardsForPotion = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.DreamAlchemy.ShardsForPotion", 10, 1, 100, "DreamAlchemy");
    m_restedBuffSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.DreamAlchemy.RestedBuffSpell", 0, "DreamAlchemy");
    m_awakeningSpell = Aurora::Settings::Get<uint32_t>(
        "Aurora.DreamAlchemy.AwakeningSpell", 0, "DreamAlchemy");

    if (!m_enabled)
    {
        LOG_INFO("DreamAlchemy", "DreamAlchemy disabled by Aurora.DreamAlchemy.Enabled");
        return true;
    }

    LOG_INFO("DreamAlchemy",
        "Dream Alchemy initialized ({} s offline, {} shards per potion, rested buff {}, awakening {})",
        m_minOfflineSecs, m_shardsForPotion, m_restedBuffSpell, m_awakeningSpell);
    return true;
}

bool DreamAlchemyManager::IsResting(Player* p) const
{
    // Rested bonus is the closest persistent "in a tavern" signal exposed by the
    // core: the player has to be in a resting area for it to accumulate.
    return p && p->GetRestBonus() > 0.0f;
}

bool DreamAlchemyManager::DrinkVial(Player* p)
{
    if (!p || !m_enabled)
        return false;

    if (!IsResting(p))
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_states[p->GetGUID()] = DreamState{ GameTime::GetGameTime().count(), 0 };

    LOG_INFO("DreamAlchemy", "{} drank a dream vial in a tavern", p->GetName());
    return true;
}

bool DreamAlchemyManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return false;

    DreamState state;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_states.find(p->GetGUID());
        if (it == m_states.end())
            return false;
        state = it->second;
    }

    int64_t const now = GameTime::GetGameTime().count();
    int64_t const offline = now - state.drunkAt;
    if (offline < static_cast<int64_t>(m_minOfflineSecs))
        return false;

    state.shards += 1;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_states[p->GetGUID()] = state;
    }

    if (m_restedBuffSpell)
        p->CastSpell(p, m_restedBuffSpell);

    LOG_INFO("DreamAlchemy", "{} woke refreshed ({} s offline, {} shards)",
        p->GetName(), offline, state.shards);
    return true;
}

bool DreamAlchemyManager::CombinePotion(Player* p)
{
    if (!p || !m_enabled)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_states.find(p->GetGUID());
    if (it == m_states.end() || it->second.shards < m_shardsForPotion)
        return false;

    it->second.shards -= m_shardsForPotion;

    if (m_awakeningSpell)
        p->CastSpell(p, m_awakeningSpell);

    LOG_INFO("DreamAlchemy", "{} brewed a potion of awakening", p->GetName());
    return true;
}

uint32_t DreamAlchemyManager::ShardCount(Player* p) const
{
    if (!p)
        return 0;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_states.find(p->GetGUID());
    return it == m_states.end() ? 0 : it->second.shards;
}

// ============================================================================
// CommandScript
// ============================================================================

class DreamAlchemyCommandScript : public CommandScript
{
public:
    DreamAlchemyCommandScript() : CommandScript("DreamAlchemyCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable dreamTable =
        {
            { "drink",  HandleDrinkCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "status", HandleStatusCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "brew",   HandleBrewCommand,   rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "dream", dreamTable }
        };

        return commandTable;
    }

private:
    static bool HandleDrinkCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The alchemy needs an in-game character.");
            return true;
        }

        if (DreamAlchemyManager::Instance().DrinkVial(p))
            handler->SendSysMessage("Dream vial drunk - now rest and log out.");
        else
            handler->SendSysMessage("You must be resting in a tavern.");

        return true;
    }

    static bool HandleStatusCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        DreamAlchemyManager& mgr = DreamAlchemyManager::Instance();
        Player* p = handler->GetPlayer();

        handler->PSendSysMessage("Dream Alchemy: shards={}/{} , resting={}",
            mgr.ShardCount(p), mgr.ShardsForPotion(), p && mgr.IsResting(p) ? "yes" : "no");
        return true;
    }

    static bool HandleBrewCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The alchemy needs an in-game character.");
            return true;
        }

        DreamAlchemyManager& mgr = DreamAlchemyManager::Instance();
        if (mgr.CombinePotion(p))
            handler->SendSysMessage("Potion of awakening brewed.");
        else
            handler->PSendSysMessage("Not enough shards: {} needed, you have {}.",
                mgr.ShardsForPotion(), mgr.ShardCount(p));

        return true;
    }
}

// ============================================================================
// PlayerScript / WorldScript
// ============================================================================

class DreamAlchemyPlayerScript : public PlayerScript
{
public:
    DreamAlchemyPlayerScript() : PlayerScript("DreamAlchemyPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        DreamAlchemyManager::Instance().OnPlayerLogin(player);
    }
}

class DreamAlchemyWorldScript : public WorldScript
{
public:
    DreamAlchemyWorldScript() : WorldScript("DreamAlchemyWorldScript") { }

    void OnStartup() override
    {
        DreamAlchemyManager::Instance().Initialize();
    }
}

void AddDreamAlchemyScripts()
{
    new DreamAlchemyCommandScript();
    new DreamAlchemyPlayerScript();
    new DreamAlchemyWorldScript();
}
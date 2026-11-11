/*
 * CafeModule - implementation
 *
 * The compensation loop: misses accumulate per raid/slot until the threshold,
 * then the player earns essences that buy an amulet of random hope. The amulet
 * and its random stat are reported rather than applied, because the buff table
 * is part of the SQL layer that ships separately.
 */
#include "CafeModule.h"

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

#include <cstdlib>

// ============================================================================
// CafeManager
// ============================================================================

CafeManager& CafeManager::Instance()
{
    static CafeManager inst;
    return inst;
}

std::string CafeManager::Key(std::string const& raid, std::string const& slot) const
{
    return raid + ":" + slot;
}

bool CafeManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Cafe.Enabled", "Cafe");
    m_missThreshold = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Cafe.MissThreshold", 10, 1, 1000, "Cafe");
    m_amuletCost = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Cafe.AmuletCost", 3, 1, 100, "Cafe");
    m_amuletDurationSecs = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Cafe.AmuletDurationSecs", 3600, 60, 86400, "Cafe");

    if (!m_enabled)
    {
        LOG_INFO("Cafe", "Cafe disabled by Aurora.Cafe.Enabled");
        return true;
    }

    LOG_INFO("Cafe", "Cafe of Broken Hearts initialized (essence every {} misses, amulet costs {}, lasts {} s)",
        m_missThreshold, m_amuletCost, m_amuletDurationSecs);
    return true;
}

uint32_t CafeManager::RegisterMiss(Player* p, std::string const& raid, std::string const& slot)
{
    if (!p || !m_enabled || raid.empty() || slot.empty())
        return 0;

    std::lock_guard<std::mutex> lock(m_mutex);

    CafeEntry& entry = m_entries[Key(raid, slot)];
    entry.misses += 1;

    uint32_t earned = 0;
    // Every full block of MissThreshold misses earns one essence; the miss
    // count is reduced by the amount that converted so the threshold rolls over.
    while (entry.misses >= m_missThreshold)
    {
        entry.misses -= m_missThreshold;
        entry.essences += 1;
        earned += 1;
    }

    LOG_INFO("Cafe", "{} missed {} / {} ({} essences total)",
        p->GetName(), raid, slot, entry.essences);

    return earned;
}

bool CafeManager::ExchangeAmulet(Player* p, std::string const& raid, std::string const& slot)
{
    if (!p || !m_enabled)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);

    CafeEntry& entry = m_entries[Key(raid, slot)];
    if (entry.essences < m_amuletCost)
        return false;

    entry.essences -= m_amuletCost;

    LOG_INFO("Cafe", "{} exchanged an amulet of random hope ({} essences left)",
        p->GetName(), entry.essences);
    return true;
}

uint32_t CafeManager::EssenceCount(std::string const& raid, std::string const& slot) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(Key(raid, slot));
    return it == m_entries.end() ? 0 : it->second.essences;
}

// ============================================================================
// CommandScript
// ============================================================================

class CafeCommandScript : public CommandScript
{
public:
    CafeCommandScript() : CommandScript("CafeCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable cafeTable =
        {
            { "fail",    HandleFailCommand,    rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "essence", HandleEssenceCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "amulet",  HandleAmuletCommand,  rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "cafe", cafeTable }
        };

        return commandTable;
    }

private:
    static bool HandleFailCommand(ChatHandler* handler, std::string raid, std::string slot)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The bar needs an in-game character.");
            return true;
        }

        CafeManager& mgr = CafeManager::Instance();
        uint32_t earned = mgr.RegisterMiss(p, raid, slot);

        handler->PSendSysMessage("A miss was registered for {} / {}.", raid, slot);
        if (earned)
            handler->PSendSysMessage("The failure condensed into {} essence(s)!", earned);
        else
            handler->PSendSysMessage("{} misses until the next essence.",
                mgr.MissThreshold());

        return true;
    }

    static bool HandleEssenceCommand(ChatHandler* handler, std::string raid, std::string slot)
    {
        CafeManager& mgr = CafeManager::Instance();
        handler->PSendSysMessage("{} / {}: {} essence(s).", raid, slot,
            mgr.EssenceCount(raid, slot));
        return true;
    }

    static bool HandleAmuletCommand(ChatHandler* handler, std::string raid, std::string slot)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The bar needs an in-game character.");
            return true;
        }

        CafeManager& mgr = CafeManager::Instance();
        if (!mgr.ExchangeAmulet(p, raid, slot))
        {
            handler->PSendSysMessage("Not enough essences: the amulet costs {}, you have {}.",
                mgr.AmuletCost(), mgr.EssenceCount(raid, slot));
            return true;
        }

        // A random stat from a fixed pool keeps the amulet non-deterministic
        // without depending on a buff table that is not imported yet.
        char const* const stats[] = { "strength", "agility", "stamina", "intellect", "spirit" };
        uint32 const roll = static_cast<uint32>(rand()) % (sizeof(stats) / sizeof(stats[0]));

        handler->PSendSysMessage("Amulet of random hope received: +{} for {} seconds.",
            stats[roll], mgr.AmuletDurationSecs());

        return true;
    }
}

// ============================================================================
// WorldScript
// ============================================================================

class CafeWorldScript : public WorldScript
{
public:
    CafeWorldScript() : WorldScript("CafeWorldScript") { }

    void OnStartup() override
    {
        CafeManager::Instance().Initialize();
    }
}

void AddCafeScripts()
{
    new CafeCommandScript();
    new CafeWorldScript();
}
/*
 * BestiaryModule - implementation
 *
 * A photograph of a mutated creature becomes a card; a full set becomes a
 * trophy. The collection is the playable part of the feature, so it lives in
 * the manager and the commands report the values the options produced.
 */
#include "BestiaryModule.h"

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

#include <algorithm>

// ============================================================================
// BestiaryManager
// ============================================================================

BestiaryManager& BestiaryManager::Instance()
{
    static BestiaryManager inst;
    return inst;
}

bool BestiaryManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Bestiary.Enabled", "Bestiary");
    m_cardSetSize = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Bestiary.CardSetSize", 10, 1, 100, "Bestiary");
    m_maxTrophies = Aurora::Settings::Bounded<uint32_t>(
        "Aurora.Bestiary.MaxTrophies", 20, 0, 1000, "Bestiary");

    if (!m_enabled)
    {
        LOG_INFO("Bestiary", "Bestiary disabled by Aurora.Bestiary.Enabled");
        return true;
    }

    LOG_INFO("Bestiary", "Bestiary module initialized ({} cards per set, {} trophies max)",
        m_cardSetSize, m_maxTrophies);
    return true;
}

bool BestiaryManager::Photograph(Player* p, std::string const& creatureType, uint32 mutation, bool& onCardComplete)
{
    onCardComplete = false;
    if (!p || !m_enabled || creatureType.empty())
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t& count = m_counts[creatureType];
    count += 1;

    LOG_INFO("Bestiary", "{} photographed a {}/{} mutation ({} of {})",
        p->GetName(), creatureType, mutation, count, m_cardSetSize);

    if (count >= m_cardSetSize)
    {
        if (m_trophies.size() < m_maxTrophies)
        {
            m_trophies.push_back(creatureType);
            onCardComplete = true;
            LOG_INFO("Bestiary", "{} completed a {} trophy ({} total)",
                p->GetName(), creatureType, static_cast<uint32>(m_trophies.size()));
        }
        count = 0; // cards are consumed by the trophy
    }

    return true;
}

std::vector<BestiaryCard> BestiaryManager::GetCards() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<BestiaryCard> cards;
    cards.reserve(m_counts.size());
    for (auto const& [type, count] : m_counts)
    {
        BestiaryCard card;
        card.creatureType = type;
        card.mutation = 0;
        card.count = count;
        cards.push_back(card);
    }
    return cards;
}

bool BestiaryManager::TrophyPlaced(std::string const& creatureType) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::find(m_trophies.begin(), m_trophies.end(), creatureType) != m_trophies.end();
}

// ============================================================================
// CommandScript
// ============================================================================

class BestiaryCommandScript : public CommandScript
{
public:
    BestiaryCommandScript() : CommandScript("BestiaryCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable bestiaryTable =
        {
            { "shoot", HandleShootCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "cards", HandleCardsCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "trophies", HandleTrophiesCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "bestiary", bestiaryTable }
        };

        return commandTable;
    }

private:
    static bool HandleShootCommand(ChatHandler* handler, std::string creatureType, Optional<uint32> mutation)
    {
        Player* p = handler->GetPlayer();
        if (!p)
        {
            handler->SendSysMessage("The camera needs an in-game character.");
            return true;
        }

        uint32_t const variant = mutation.value_or(1);
        bool completed = false;
        if (BestiaryManager::Instance().Photograph(p, creatureType, variant, completed))
        {
            handler->PSendSysMessage("Snapshot taken: {} mutation {}.", creatureType, variant);
            if (completed)
                handler->PSendSysMessage("Card set complete - a trophy is now available!");
        }
        else
        {
            handler->SendSysMessage("Snapshot failed (empty type or module disabled).");
        }

        return true;
    }

    static bool HandleCardsCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto cards = BestiaryManager::Instance().GetCards();
        if (cards.empty())
        {
            handler->SendSysMessage("No monster cards yet.");
            return true;
        }

        handler->PSendSysMessage("Monster cards (set size {}):", BestiaryManager::Instance().CardSetSize());
        for (auto const& card : cards)
            handler->PSendSysMessage("- {}: {} / {}", card.creatureType, card.count,
                BestiaryManager::Instance().CardSetSize());

        return true;
    }

    static bool HandleTrophiesCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        auto cards = BestiaryManager::Instance().GetCards();
        std::vector<std::string> trophies;
        for (auto const& card : cards)
            if (BestiaryManager::Instance().TrophyPlaced(card.creatureType))
                trophies.push_back(card.creatureType);

        if (trophies.empty())
        {
            handler->SendSysMessage("No trophies placed yet.");
            return true;
        }

        handler->PSendSysMessage("Trophies placed:");
        for (auto const& t : trophies)
            handler->PSendSysMessage("- {}", t);

        return true;
    }
};

// ============================================================================
// WorldScript
// ============================================================================

class BestiaryWorldScript : public WorldScript
{
public:
    BestiaryWorldScript() : WorldScript("BestiaryWorldScript") { }

    void OnStartup() override
    {
        BestiaryManager::Instance().Initialize();
    }
};

void AddBestiaryScripts()
{
    new BestiaryCommandScript();
    new BestiaryWorldScript();
}
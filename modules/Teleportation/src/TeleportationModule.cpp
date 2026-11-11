#include "TeleportationModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>

#include <iterator>

TeleportationManager::TeleportationManager() {}

TeleportationManager::~TeleportationManager() {}

bool TeleportationManager::Initialize()
{
    m_enabled = Aurora::Settings::Enabled("Aurora.Teleportation.Enabled", "Teleportation");

    // Capital destinations are only a convenience: a server that keeps its
    // own teleport table in the database turns this off.
    bool loadDefaults = Aurora::Settings::Enabled("Aurora.Teleportation.LoadDefaults", "Teleportation", true);

    if (!m_enabled)
    {
        LOG_INFO("Teleportation", "Teleportation disabled by Aurora.Teleportation.Enabled");
        return true;
    }

    if (loadDefaults)
    {
        // Which of the built-in capitals actually become destinations. An empty
        // list would leave the module running with nothing to teleport to, so
        // the option is a filter over the table below, not a replacement for it.
        std::vector<std::string> capitals = Aurora::Settings::List("Aurora.Teleportation.Capitals",
            "Stormwind,Orgrimmar,Ironforge,Undercity,Thunder Bluff", "Teleportation");

        struct DefaultCapital
        {
            char const* name;
            float x, y, z;
            uint32_t mapId;
            char const* zone;
        };

        static DefaultCapital const table[] = {
            { "Stormwind",     -8905.0f,  562.0f,   94.0f, 0, "Stormwind City" },
            { "Orgrimmar",      1600.0f, -4400.0f,   10.0f, 1, "Orgrimmar City" },
            { "Ironforge",     -4980.0f,  -880.0f,  500.0f, 0, "Ironforge City" },
            { "Undercity",      1830.0f,   240.0f,   61.0f, 0, "Undercity" },
            { "Thunder Bluff", -1280.0f,   120.0f,  130.0f, 1, "Thunder Bluff" },
        };

        uint32 skipped = 0;
        for (DefaultCapital const& capital : table)
        {
            if (!Aurora::Settings::Contains(capitals, capital.name))
            {
                ++skipped;
                continue;
            }

            AddLocation(capital.name, capital.x, capital.y, capital.z, capital.mapId, capital.zone);
        }

        if (skipped)
            LOG_INFO("Teleportation", "{} of {} default capitals left out by Aurora.Teleportation.Capitals",
                skipped, static_cast<uint32>(std::size(table)));
    }

    LOG_INFO("Teleportation", "Teleportation ready with {} locations", static_cast<uint32>(m_locations.size()));
    return true;
}

bool TeleportationManager::AddLocation(const std::string& name, float x, float y, float z, uint32_t mapId, const std::string& description)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    TeleportLocation loc;
    loc.name = name;
    loc.x = x;
    loc.y = y;
    loc.z = z;
    loc.mapId = mapId;
    loc.description = description;
    m_locations[name] = loc;
    LOG_INFO("Teleportation", "Added location: {}", name);
    return true;
}

bool TeleportationManager::RemoveLocation(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_locations.find(name);
    if (it == m_locations.end()) return false;
    m_locations.erase(it);
    LOG_INFO("Teleportation", "Removed location: {}", name);
    return true;
}

bool TeleportationManager::TeleportPlayer(Player* pPlayer, const std::string& locationName)
{
    if (!pPlayer)
        return false;

    TeleportLocation loc{};
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_locations.find(locationName);
        if (it == m_locations.end())
            return false;

        loc = it->second;
    }

    LOG_INFO("Teleportation", "Teleporting player {} to {}", pPlayer->GetName(), loc.name);

    // Without this the command only logged a teleport and the player stayed
    // where they were, which made the whole registry cosmetic.
    pPlayer->TeleportTo(loc.mapId, loc.x, loc.y, loc.z, 0.0f);
    return true;
}

std::vector<TeleportLocation> TeleportationManager::GetLocations() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TeleportLocation> list;
    for (const auto& pair : m_locations) {
        list.push_back(pair.second);
    }
    return list;
}

bool TeleportationManager::LocationExists(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_locations.find(name) != m_locations.end();
}

// ============================================================================
// WorldScript - the module options live in configs/modules/, which the core
// loads only after worldserver.conf, so they are read here rather than in a
// static initializer.
// ============================================================================

class TeleportationWorldScript : public WorldScript
{
public:
    TeleportationWorldScript() : WorldScript("TeleportationWorldScript") { }

    void OnStartup() override
    {
        TeleportationManager::Instance().Initialize();
    }
}


// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddTeleportationScripts()
{
    new TeleportationWorldScript();
}

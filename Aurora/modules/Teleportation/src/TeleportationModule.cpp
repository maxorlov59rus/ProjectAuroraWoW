#include "TeleportationModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

TeleportationManager::TeleportationManager() {}

TeleportationManager::~TeleportationManager() {}

bool TeleportationManager::Initialize()
{
    AddLocation("Stormwind", -8905.0f, 562.0f, 94.0f, 0, "Stormwind City");
    AddLocation("Orgrimmar", 1600.0f, -4400.0f, 10.0f, 1, "Orgrimmar City");
    AddLocation("Ironforge", -4980.0f, -880.0f, 500.0f, 0, "Ironforge City");
    AddLocation("Undercity", 1830.0f, 240.0f, 61.0f, 0, "Undercity");
    AddLocation("Thunder Bluff", -1280.0f, 120.0f, 130.0f, 1, "Thunder Bluff");
    LOG_INFO("Teleportation", "Teleportation system initialized with %d locations", (int)m_locations.size());
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
    LOG_INFO("Teleportation", "Added location: %s", name.c_str());
    return true;
}

bool TeleportationManager::RemoveLocation(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_locations.find(name);
    if (it == m_locations.end()) return false;
    m_locations.erase(it);
    LOG_INFO("Teleportation", "Removed location: %s", name.c_str());
    return true;
}

bool TeleportationManager::TeleportPlayer(Player* pPlayer, const std::string& locationName)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_locations.find(locationName);
    if (it == m_locations.end()) return false;
    LOG_INFO("Teleportation", "Teleporting player %s to %s", pPlayer->GetName().c_str(), locationName.c_str());
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
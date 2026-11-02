/*
 * TeleportationModule - teleportation module for AzerothCore (WotLK)
 *
 * Provides a teleport destination registry and moves players to those points.
 */

#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
class WorldSession;
class Player;
class ChatHandler;

// A single teleport destination
struct TeleportLocation
{
    std::string name;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    uint32_t mapId = 0;
    std::string description;
};

// Global teleportation manager
class TeleportationManager
{
public:
    static TeleportationManager& Instance()
    {
        static TeleportationManager inst;
        return inst;
    }

    // Loads the options and, unless disabled, the capital destinations.
    bool Initialize();

    // Add a teleport destination
    bool AddLocation(const std::string& name, float x, float y, float z, uint32_t mapId, const std::string& description = "");

    // Remove a teleport destination
    bool RemoveLocation(const std::string& name);

    // Move a player to a destination
    bool TeleportPlayer(Player* pPlayer, const std::string& locationName);

    // All known destinations
    std::vector<TeleportLocation> GetLocations() const;

    // Whether a destination is registered
    bool LocationExists(const std::string& name) const;

    bool IsEnabled() const { return m_enabled; }

private:
    TeleportationManager();
    ~TeleportationManager();

    std::map<std::string, TeleportLocation> m_locations;
    mutable std::mutex m_mutex;
    bool m_enabled = true;
};

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeTeleportationModule();
    __declspec(dllexport) void __stdcall FinalizeTeleportationModule();
}

/**
 * AuroraSkins.cpp - Skins class implementation.
 */
#include <AuroraPCH.h>

#include <AuroraSkins.h>
#include <AuroraLogging.h>

namespace Aurora {

Skins::Skins()
{
}

Skins::~Skins()
{
}

bool Skins::Initialize()
{
    Logging::Info("AuroraSkins", "Skins subsystem initialised");
    return true;
}

void Skins::Shutdown()
{
    Logging::Info("AuroraSkins", "Skins subsystem shut down");
}

std::string Skins::GetSkinFor(uint32_t /*characterId*/)
{
    // Default skin for a character. Loaded from a removable table in DB, which
    // today means the configured default (Aurora.Skins.Default).
    return s_defaultSkin;
}

void Skins::SetDefaultSkin(const std::string& skinId)
{
    s_defaultSkin = skinId.empty() ? "default" : skinId;
}

std::string const& Skins::GetDefaultSkin()
{
    return s_defaultSkin;
}

void Skins::RegisterSkinListener(std::function<void(uint32_t, const std::string&)> listener)
{
    s_listeners.push_back(std::move(listener));
}

std::vector<std::function<void(uint32_t, const std::string&)>> Skins::s_listeners;
std::string Skins::s_defaultSkin = "default";

} // namespace Aurora

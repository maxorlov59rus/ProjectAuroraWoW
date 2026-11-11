/**
 * AuroraInterface.cpp - Interface class implementation.
 */
#include <AuroraPCH.h>

#include <AuroraInterface.h>
#include <AuroraLogging.h>

namespace Aurora {

Interface::Interface()
    : m_initialized(false)
{
}

Interface::~Interface()
{
    Shutdown();
}

bool Interface::Initialize()
{
    m_initialized = true;
    Logging::Info("AuroraInterface", "Interface subsystem initialised");
    return true;
}

void Interface::Shutdown()
{
    m_initialized = false;
}

void Interface::RegisterTabs()
{
    // Register UI tabs/menus in the client. Server-side stub.
}

bool Interface::SendTextMessageTo(uint32_t /*characterId*/, const std::string& /*message*/)
{
    return false;
}

bool Interface::ClientHasAuroraUI()
{
    // Returns false until the client with Aurora UI is connected.
    return false;
}

} // namespace Aurora

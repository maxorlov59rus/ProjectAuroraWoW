/**
 * AuroraInterface
 *
 * Custom interface / UI layer for Aurora. The class demonstrates how to
 * expose Aurora-specific data to the client.
 *
 * Implementation notes:
 *  - Do not embed raw WoW client files here. The client expects a
 *    standard Wotlk layout. UI modifications are handled by the client
 *    upgrade project.
 *  - Use this class to store custom interface state and to prepare data
 *    packets that Aurora-specific UI modules will send to clients.
 */
#pragma once

#include <string>

namespace Aurora {

class Interface
{
public:
    Interface();
    ~Interface();

    // Called on server start: load interface configuration, register packets.
    bool Initialize();

    // Called on server stop: flush pending data, cleanup.
    void Shutdown();

    // Register a UI tab/menu item in the client (server-side stub).
    // By default does nothing; override in a derived Aurora module.
    virtual void RegisterTabs();

    // Example: send a text message to the named client. Returns false if
    // the client is not connected.
    static bool SendTextMessageTo(uint32_t characterId, const std::string& message);

    // Checks whether the client is using the Aurora UI pack.
    static bool ClientHasAuroraUI(); // returns false by default

private:
    bool m_initialized = false;
};

} // namespace Aurora

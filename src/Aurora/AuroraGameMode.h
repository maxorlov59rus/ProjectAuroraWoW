/**
 * AuroraGameMode
 *
 * Base class for Aurora-specific game modes. Derived classes are plugged
 * into the module loader (AuroraGameModes) and can override world/server
 * behaviour at runtime.
 *
 * Design notes:
 *  - Keep the constructor/destructor trivial: the module loader will create
 *    and destroy instances.
 *  - Do not block the world thread. All heavy work must be deferred to
 *    dedicated threads.
 *  - Use logger (see AuroraLogging.h) for diagnostics.
 */
#pragma once

#include <string>

namespace Aurora {

class GameMode
{
public:
    // Unique name; used to identify the mode in logs and configuration
    virtual const std::string& GetName() const = 0;

    // Called once when the module is loaded. Return true on success.
    virtual bool Initialize() = 0;

    // Called when the module is unloaded. Use for cleanup.
    virtual void Shutdown() = 0;

    // Heartbeat: called periodically (interval is defined by each mode).
    // Override with empty implementation if the mode does not need heartbeat.
    virtual void OnTick() {}
};

} // namespace Aurora

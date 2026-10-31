/**
 * AuroraSkinsModule — loadable AzerothCore module
 *
 * Entry point for the skins module. Registered through
 * RegisterAuroraSkins(), which the AzerothCore module loader calls when
 * the module is loaded.
 */
#include <AuroraPCH.h>

#include <AuroraSkins.h>
#include <AuroraLogging.h>

namespace Aurora {

class SkinsModule
{
public:
    SkinsModule() = default;
    ~SkinsModule() = default;

    bool Initialize()
    {
        Logging::Info("AuroraSkins", "Skins module loaded");
        return true;
    }

    void Shutdown()
    {
        Logging::Info("AuroraSkins", "Skins module unloaded");
    }

    static SkinsModule& Instance()
    {
        static SkinsModule instance;
        return instance;
    }
};

extern "C" void RegisterAuroraSkins()
{
    SkinsModule::Instance().Initialize();
}

extern "C" void UnregisterAuroraSkins()
{
    SkinsModule::Instance().Shutdown();
}

} // namespace Aurora

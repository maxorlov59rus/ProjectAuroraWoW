/**
 * AuroraGameModesModule — loadable AzerothCore module
 *
 * Entry point for the game-modes module. Registered through
 * RegisterAuroraGameModes(), which the AzerothCore module loader calls
 * when the module is loaded.
 */
#include <AuroraPCH.h>

#include <AuroraGameMode.h>
#include <AuroraLogging.h>

namespace Aurora {

class GameModesModule
{
public:
    GameModesModule() = default;
    ~GameModesModule() = default;

    bool Initialize()
    {
        Logging::Info("AuroraGameModes", "Game-modes module loaded");
        return true;
    }

    void Shutdown()
    {
        Logging::Info("AuroraGameModes", "Game-modes module unloaded");
    }

    static GameModesModule& Instance()
    {
        static GameModesModule instance;
        return instance;
    }
};

extern "C" void RegisterAuroraGameModes()
{
    GameModesModule::Instance().Initialize();
}

extern "C" void UnregisterAuroraGameModes()
{
    GameModesModule::Instance().Shutdown();
}

} // namespace Aurora

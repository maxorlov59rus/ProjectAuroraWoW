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
#include <AuroraModuleSettings.h>

#include "StringFormat.h"

#include <ScriptMgr.h>
#include <WorldScript.h>

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


// ============================================================================
// AzerothCore integration
//
// The Aurora layer itself stays core-free; this script is the only place that
// talks to the core, and its only job is to apply the module options from
// configs/modules/aurora_gamemodes.conf once they are readable (OnStartup, not static init).
// ============================================================================

#include <ScriptMgr.h>
#include <WorldScript.h>

namespace {

    class AuroraGameModesWorldScript : public WorldScript
    {
    public:
        AuroraGameModesWorldScript() : WorldScript("AuroraGameModesWorldScript") { }

        void OnStartup() override
        {
            if (!Aurora::Settings::Enabled("Aurora.GameModes.Enabled", "AuroraGameModes"))
            {
                Logging::Info("AuroraGameModes", "disabled by Aurora.GameModes.Enabled");
                return;
            }

            std::string mode = Aurora::Settings::Get<std::string>(
                "Aurora.GameModes.Default", "standard", "AuroraGameModes");

            Logging::Info("AuroraGameModes", Acore::StringFormat(
                "default game mode '{}' (known: standard, rest, test)", mode));
            GameModesModule::Instance().Initialize();
        }
    };



} // namespace Aurora

} // namespace Aurora

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddAuroraGameModesScripts()
{
    new Aurora::AuroraGameModesWorldScript();
}

/**
 * AuroraGameModesModule - loadable AzerothCore module
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
}

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

            // Allowed is the set of modes the server is willing to run; a Default
            // outside it would silently start an unconfigured mode, so fall back
            // to the first allowed one instead.
            std::vector<std::string> allowed = Aurora::Settings::List(
                "Aurora.GameModes.Allowed", "standard,rest,test", "AuroraGameModes");

            std::string mode = Aurora::Settings::Get<std::string>(
                "Aurora.GameModes.Default", "standard", "AuroraGameModes");

            if (!Aurora::Settings::Contains(allowed, mode))
            {
                if (allowed.empty())
                {
                    Logging::Warn("AuroraGameModes", Acore::StringFormat(
                        "Aurora.GameModes.Allowed is empty, no game mode can start; default '{}' ignored", mode));
                    GameModesModule::Instance().Initialize();
                    return;
                }

                Logging::Warn("AuroraGameModes", Acore::StringFormat(
                    "default mode '{}' is not in Aurora.GameModes.Allowed, using '{}'", mode, allowed.front()));
                mode = allowed.front();
            }

            std::string allowedList;
            for (std::string const& name : allowed)
                allowedList += (allowedList.empty() ? "" : ",") + name;

            Logging::Info("AuroraGameModes", Acore::StringFormat(
                "default game mode '{}' (allowed: {})", mode, allowedList));
            GameModesModule::Instance().Initialize();
        }
    };



} // namespace

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

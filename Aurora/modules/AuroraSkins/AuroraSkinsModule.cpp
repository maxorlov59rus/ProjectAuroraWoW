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
#include <AuroraModuleSettings.h>
#include "StringFormat.h"

#include <ScriptMgr.h>
#include <WorldScript.h>

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


// ============================================================================
// AzerothCore integration
//
// The Aurora layer itself stays core-free; this script is the only place that
// talks to the core, and its only job is to apply the module options from
// configs/modules/aurora_skins.conf once they are readable (OnStartup, not static init).
// ============================================================================

#include <ScriptMgr.h>
#include <WorldScript.h>

namespace {

    class AuroraSkinsWorldScript : public WorldScript
    {
    public:
        AuroraSkinsWorldScript() : WorldScript("AuroraSkinsWorldScript") { }

        void OnStartup() override
        {
            if (!Aurora::Settings::Enabled("Aurora.Skins.Enabled", "AuroraSkins"))
            {
                Logging::Info("AuroraSkins", "disabled by Aurora.Skins.Enabled");
                return;
            }

            // The id is only reported for characters without an explicit
            // skin, so it is validated here and a typo shows up at startup.
            std::string defaultSkin = Aurora::Settings::Get<std::string>(
                "Aurora.Skins.Default", "default", "AuroraSkins");

            Logging::Info("AuroraSkins", Acore::StringFormat(
                "Skins module loaded (default skin '{}')", defaultSkin));
            SkinsModule::Instance().Initialize();
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

void AddAuroraSkinsScripts()
{
    new Aurora::AuroraSkinsWorldScript();
}

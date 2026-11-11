/**
 * AuroraSkillsModule - loadable AzerothCore module
 *
 * Entry point for the skills module. Registered through
 * RegisterAuroraSkills(), which the AzerothCore module loader calls when
 * the module is loaded.
 */
#include <AuroraPCH.h>

#include <AuroraSkills.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>
#include "StringFormat.h"

#include <ScriptMgr.h>
#include <WorldScript.h>

namespace Aurora {

class SkillsModule
{
public:
    SkillsModule() = default;
    ~SkillsModule() = default;

    bool Initialize()
    {
        Logging::Info("AuroraSkills", "Skills module loaded");
        return true;
    }

    void Shutdown()
    {
        Logging::Info("AuroraSkills", "Skills module unloaded");
    }

    static SkillsModule& Instance()
    {
        static SkillsModule instance;
        return instance;
    }
};

extern "C" void RegisterAuroraSkills()
{
    SkillsModule::Instance().Initialize();
}

extern "C" void UnregisterAuroraSkills()
{
    SkillsModule::Instance().Shutdown();
}


// ============================================================================
// AzerothCore integration
//
// The Aurora layer itself stays core-free; this script is the only place that
// talks to the core, and its only job is to apply the module options from
// configs/modules/aurora_skills.conf once they are readable (OnStartup, not static init).
// ============================================================================

#include <ScriptMgr.h>
#include <WorldScript.h>

namespace {

    class AuroraSkillsWorldScript : public WorldScript
    {
    public:
        AuroraSkillsWorldScript() : WorldScript("AuroraSkillsWorldScript") { }

        void OnStartup() override
        {
            if (!Aurora::Settings::Enabled("Aurora.Skills.Enabled", "AuroraSkills"))
            {
                Logging::Info("AuroraSkills", "disabled by Aurora.Skills.Enabled");
                return;
            }

            // LogHooks makes the two Skills hooks write a line per call, so the
            // option has to reach the layer that owns the hooks.
            bool logHooks = Aurora::Settings::Enabled("Aurora.Skills.LogHooks", "AuroraSkills", false);
            Skills::SetLogHooks(logHooks);

            Logging::Info("AuroraSkills", Acore::StringFormat(
                "Skills module loaded (hook logging: {})", logHooks ? "on" : "off"));
            SkillsModule::Instance().Initialize();
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

void AddAuroraSkillsScripts()
{
    new Aurora::AuroraSkillsWorldScript();
}

/**
 * AuroraSkillsModule — loadable AzerothCore module
 *
 * Entry point for the skills module. Registered through
 * RegisterAuroraSkills(), which the AzerothCore module loader calls when
 * the module is loaded.
 */
#include <AuroraPCH.h>

#include <AuroraSkills.h>
#include <AuroraLogging.h>

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

} // namespace Aurora

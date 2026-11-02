/**
 * AuroraSkills.cpp - Skills class implementation.
 */
#include <AuroraPCH.h>

#include <AuroraSkills.h>
#include <AuroraLogging.h>

namespace Aurora {

Skills::Skills()
{
}

Skills::~Skills()
{
}

bool Skills::Initialize()
{
    Logging::Info("AuroraSkills", "Skills subsystem initialised");
    return true;
}

void Skills::Shutdown()
{
    Logging::Info("AuroraSkills", "Skills subsystem shut down");
}

void Skills::OnExperienceGained(uint32_t characterId, int32_t amount)
{
    // Hook point: honor Aurora skill/EXP rules here.
    if (s_logHooks)
        Logging::Info("AuroraSkills", "OnExperienceGained: character " + std::to_string(characterId)
            + " gained " + std::to_string(amount));
}

void Skills::OnSkillUpdated(uint32_t characterId, uint32_t skillId)
{
    // Hook point: validate/override skill update here.
    if (s_logHooks)
        Logging::Info("AuroraSkills", "OnSkillUpdated: character " + std::to_string(characterId)
            + " skill " + std::to_string(skillId));
}

void Skills::SetLogHooks(bool enabled)
{
    s_logHooks = enabled;
}

bool Skills::LogHooks()
{
    return s_logHooks;
}

bool Skills::s_logHooks = false;

} // namespace Aurora

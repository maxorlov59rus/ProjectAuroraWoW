/**
 * AuroraSkills
 *
 * Entry point for Aurora-specific skills. This module is loaded by the
 * AzerothCore module loader and can register custom skill handlers,
 * override experience calculations or adjust skill limits.
 *
 * Reinvention guidance (do not implement in this stub):
 *  - Register new skills in `character_db2.sql` and pull them via
 *    AuroraTools::DatabaseManager.
 *  - If a skill should behave differently from vanilla WoW, patch the
 *    existing AzerothCore skill handler and write a condition check here.
 *  - Never modify server packet logic in this file; keep it as an
 *    orchestrator for your custom rules.
 */
#pragma once

#include <string>

namespace Aurora {

class Skills
{
public:
    Skills();
    ~Skills();

    // Called once when the module is loaded
    bool Initialize();

    // Called when the module is unloaded
    void Shutdown();

    // Example hook: called when a character gains an experience point.
    // Signature must match the C-server callback signature expected by
    // AzerothCore's module loader. Adjust to the actual AzerothCore API.
    static void OnExperienceGained(uint32_t characterId, int32_t amount);

    // Example hook: called when a skill is learned/updated.
    static void OnSkillUpdated(uint32_t characterId, uint32_t skillId);

    // LogHooks (Aurora.Skills.LogHooks) turns the two hooks above into log lines.
    // They run on every experience gain, so it is off by default and only useful
    // while tracing why a skill or xp amount looks wrong.
    static void SetLogHooks(bool enabled);
    static bool LogHooks();

private:
    // Any per-instance state goes here (no global state in modules).
    static bool s_logHooks;
};

} // namespace Aurora

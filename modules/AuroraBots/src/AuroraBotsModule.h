#pragma once

// AuroraBots module - bot registry with chat/console commands.
//
// The registry lives inside the module's anonymous namespace (AuroraBotsModule.cpp);
// this header only declares the entry points the AzerothCore loader needs.
// Source files are copied into AC/modules/ by scripts/migrate_to_ac.py.

namespace AuroraBots
{
    class WorldScriptAdapter;   // defined in AuroraBotsModule.cpp
}

// Called by the generated ModulesLoader.cpp from sScriptMgr->Initialize().
void AddAuroraBotsScripts();

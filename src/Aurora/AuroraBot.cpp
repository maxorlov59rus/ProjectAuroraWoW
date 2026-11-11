/*
 * AuroraBot.cpp - реализация класса бота.
 * Наследуется от Player (AzerothCore).
 */

#include "AuroraBot.h"

#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <ObjectGuid.h>

AuroraBot::AuroraBot() : Player(nullptr)
{
}

AuroraBot::~AuroraBot() = default;

bool AuroraBot::Initialize(const std::string& name,
                           const std::string& className,
                           const std::string& race,
                           const std::string& luaScript)
{
    if (m_initialized)
        return false;

    m_info.name = name;
    m_info.className = className;
    m_info.race = race;
    m_info.luaScript = luaScript;

    if (!luaScript.empty())
    {
        m_luaScriptPath = "scripts/bots/" + luaScript + ".lua";
        if (!LoadBotScript(this))
        {
            Aurora::Logging::Warn("AuroraBot", "Failed to load Lua script: " + m_luaScriptPath);
        }
    }

    m_initialized = true;
    Aurora::Logging::Info("AuroraBot", "Bot initialized: " + name + " (class: " + className + ", race: " + race + ")");
    return true;
}

void AuroraBot::Shutdown()
{
    m_initialized = false;
    Aurora::Logging::Info("AuroraBot", "Bot shutdown: " + m_info.name);
}

void AuroraBot::OnUpdate(uint32 diff)
{
    if (!m_initialized)
        return;

    RunBotScriptEvent(this, "OnBotUpdate", diff);

    OnMove(diff);
    OnSpellCast(diff);
}

void AuroraBot::OnMove(uint32 /*diff*/)
{
}

void AuroraBot::OnSpellCast(uint32 /*diff*/)
{
}

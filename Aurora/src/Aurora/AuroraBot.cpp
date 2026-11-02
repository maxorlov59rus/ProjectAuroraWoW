/*
 * AuroraBot.cpp - реализация класса бота.
 * Наследуется от Player (AzerothCore).
*/

#include "AuroraBot.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>
#include <Guid.h>

AuroraBot::AuroraBot() = default;
AuroraBot::~AuroraBot() = default;

bool AuroraBot::Initialize(const std::string& name,
                           const std::string& className,
                           const std::string& race,
                           const std::string& luaScript)
{
    if (m_initialized) return false;

    m_info.name = name;
    m_info.className = className;
    m_info.race = race;
    m_info.luaScript = luaScript;

    m_luaScriptPath = "scripts/bots/" + luaScript + ".lua";

    // Попытка загрузить Lua-скрипт
    if (!m_luaScriptPath.empty()) {
        if (!LoadBotScript(this)) {
            LOG_WARN("AuroraBot", "Failed to load Lua script: %s", m_luaScriptPath.c_str());
            // Не считаем это фатальной ошибкой, скрипт может быть опциональным.
        }
    }

    m_initialized = true;
    LOG_INFO("AuroraBot", "Bot initialized: %s (class: %s, race: %s)", name.c_str(), className.c_str(), race.c_str());
    return true;
}

void AuroraBot::Shutdown()
{
    m_initialized = false;
    // Здесь можно отписывать хуки, сохранять состояние и т.д.
    LOG_INFO("AuroraBot", "Bot shutdown: %s", m_info.name.c_str());
}

void AuroraBot::OnUpdate(uint32 diff)
{
    // НОВОЕ: Выполнение Lua-события бота
    RunBotScriptEvent(this, "OnBotUpdate", diff);

    if (!m_initialized) return;

    // Базовая логика движения/комбата (можно переопределять в Lua)
    OnMove(diff);
    OnSpellCast(diff);
}

void AuroraBot::OnMove(uint32 diff)
{
    // Базовая логика движения (можно переопределять в Lua)
    // Например, простая патрульная точка
    LOG_DEBUG("AuroraBot", "Bot %s moving (diff=%u)", m_info.name.c_str(), diff);
}

void AuroraBot::OnSpellCast(uint32 diff)
{
    // Базовая логика_cast (можно переопределить)
    LOG_DEBUG("AuroraBot", "Bot %s considering spell cast (diff=%u)", m_info.name.c_str(), diff);
}

bool AuroraBot::GetInfo() const { return m_info; }

} // namespace Aurora
void AuroraBot::OnUpdate(uint32 diff)
{
    // NEW: Run Lua script event
    RunBotScriptEvent(this, "OnBotUpdate", diff);

    if (!m_initialized) return;

    OnMove(diff);
    OnSpellCast(diff);
}

void AuroraBot::OnUpdate(uint32 diff)
{
    // NEW: Run Lua script event
    RunBotScriptEvent(this, "OnBotUpdate", diff);

    if (!m_initialized) return;

    OnMove(diff);
    OnSpellCast(diff);
}

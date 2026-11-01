/*
 * WeatherSystem module — implementation
 *
 * Map weather control: set a weather type or trigger a timed storm.
 *
 * This module registers one AzerothCore script:
 *   - WeatherSystemCommandScript : CommandScript (chat commands)
 */

#include "WeatherSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// WeatherSystemManager — implementation
// ============================================================================

WeatherSystemManager::WeatherSystemManager() = default;

WeatherSystemManager::~WeatherSystemManager() = default;

WeatherSystemManager& WeatherSystemManager::Instance()
{
    static WeatherSystemManager inst;
    return inst;
}

bool WeatherSystemManager::Initialize()
{
    LOG_INFO("WeatherSystem", "WeatherSystem module initialized");
    return true;
}

void WeatherSystemManager::OnPlayerLogin(Player* p)
{
    if (!p)
        return;
    LOG_INFO("WeatherSystem", "Player {} logged in — weather system available", p->GetName());
}

void WeatherSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p)
        return;
    LOG_INFO("WeatherSystem", "Player {} leveled up", p->GetName());
}


void WeatherSystemManager::SetWeather(uint32_t mapId, uint32_t weatherType)
{
    // TODO: push the change to the live map once Map::SetWeather is wired up.
    LOG_INFO("WeatherSystem", "Set weather type {} on map {}", weatherType, mapId);
}

void WeatherSystemManager::TriggerStorm(uint32_t mapId, uint32_t duration)
{
    LOG_INFO("WeatherSystem", "Triggered storm on map {} for {} seconds", mapId, duration);
}

// ============================================================================
// CommandScript
// ============================================================================

class WeatherSystemCommandScript : public CommandScript
{
public:
    WeatherSystemCommandScript() : CommandScript("WeatherSystemCommandScript") { }

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {
        using namespace Acore::ChatCommands;

        static ChatCommandTable weatherTable =
        {
            { "set", HandleSetWeatherCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "storm", HandleTriggerStormCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
            { "info", HandleWeatherInfoCommand, rbac::RBAC_PERM_COMMAND_GM, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "weather", weatherTable }
        };

        return commandTable;
    }

private:
    // ---------- .weather set ----------
    static bool HandleSetWeatherCommand(ChatHandler* handler, uint32 mapId, uint32 weatherType)
    {
        handler->PSendSysMessage("Weather type {} set on map {}.", mapId, weatherType);
        return true;
    }

    // ---------- .weather storm ----------
    static bool HandleTriggerStormCommand(ChatHandler* handler, uint32 mapId, uint32 duration)
    {
        handler->PSendSysMessage("Storm triggered on map {} for {} seconds.", mapId, duration);
        return true;
    }

    // ---------- .weather info ----------
    static bool HandleWeatherInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        handler->SendSysMessage("Weather info command received.");
        return true;
    }

};

// ============================================================================
// Module entry points
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeWeatherSystemModule()
    {
        WeatherSystemManager::Instance().Initialize();
        LOG_INFO("WeatherSystem", "WeatherSystem module initialized (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeWeatherSystemModule()
    {
        LOG_INFO("WeatherSystem", "WeatherSystem module finalized");
    }
}

namespace
{
    WeatherSystemCommandScript sWeatherSystemCommandScript;
}

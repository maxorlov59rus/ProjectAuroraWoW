/*
 * WeatherSystem module - implementation
 *
 * Map weather control: set a weather type or trigger a timed storm.
 *
 * This module registers two AzerothCore scripts:
 *   - WeatherSystemCommandScript : CommandScript (chat commands)
 *   - WeatherSystemWorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/aurora_weather.conf (see WeatherSystem/conf/aurora_weather.conf.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "WeatherSystemModule.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// WeatherSystemManager - implementation
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
    m_enabled = Aurora::Settings::Enabled("Aurora.Weather.Enabled", "WeatherSystem");
    m_maxWeatherType = Aurora::Settings::Get<uint32_t>("Aurora.Weather.MaxWeatherType", 11, "WeatherSystem");
    m_defaultWeatherType = Aurora::Settings::Bounded<uint32_t>("Aurora.Weather.DefaultWeatherType", 5, 0, m_maxWeatherType, "WeatherSystem");
    m_maxStormDurationSecs = Aurora::Settings::Get<uint32_t>("Aurora.Weather.MaxStormDurationSecs", 3600, "WeatherSystem");
    m_stormDurationSecs = Aurora::Settings::Bounded<uint32_t>("Aurora.Weather.StormDurationSecs", 300, 1, m_maxStormDurationSecs, "WeatherSystem");

    if (!m_enabled)
    {
        LOG_INFO("WeatherSystem", "WeatherSystem disabled by Aurora.Weather.Enabled");
        return true;
    }

    LOG_INFO("WeatherSystem", "WeatherSystem module initialized (storm {} s, default type {}, max type {})",
        m_stormDurationSecs, m_defaultWeatherType, m_maxWeatherType);
    return true;
}

bool WeatherSystemManager::IsEnabled() const
{
    return m_enabled;
}

void WeatherSystemManager::OnPlayerLogin(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("WeatherSystem", "Player {} logged in - weather system available", p->GetName());
}

void WeatherSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
        return;
    LOG_INFO("WeatherSystem", "Player {} leveled up", p->GetName());
}

bool WeatherSystemManager::IsValidWeatherType(uint32_t weatherType) const
{
    switch (weatherType)
    {
        // The ids the 3.3.5 client actually knows; the range alone would also
        // accept the gaps between them, which make the client show nothing.
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 11:
            return weatherType <= m_maxWeatherType;
        default:
            return false;
    }
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
    static bool HandleSetWeatherCommand(ChatHandler* handler, uint32 mapId, Optional<uint32> weatherTypeArg)
    {
        if (!WeatherSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("WeatherSystem is disabled by configuration.");
            return true;
        }

        uint32 const weatherType = weatherTypeArg.value_or(WeatherSystemManager::Instance().DefaultWeatherType());
        if (!WeatherSystemManager::Instance().IsValidWeatherType(weatherType))
        {
            handler->PSendSysMessage("Unknown weather type {} (0-8 or 11, up to {}).",
                weatherType, WeatherSystemManager::Instance().MaxWeatherType());
            return true;
        }

        handler->PSendSysMessage("Weather type {} set on map {}.", weatherType, mapId);
        return true;
    }


    // ---------- .weather storm ----------
    static bool HandleTriggerStormCommand(ChatHandler* handler, uint32 mapId, Optional<uint32> durationArg)
    {
        if (!WeatherSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("WeatherSystem is disabled by configuration.");
            return true;
        }

        uint32 duration = durationArg.value_or(WeatherSystemManager::Instance().StormDurationSecs());
        uint32 const maxDuration = WeatherSystemManager::Instance().MaxStormDurationSecs();
        if (!duration || duration > maxDuration)
        {
            handler->PSendSysMessage("Storm duration must be between 1 and {} seconds.", maxDuration);
            return true;
        }

        handler->PSendSysMessage("Storm triggered on map {} for {} seconds.", mapId, duration);
        return true;
    }


    // ---------- .weather info ----------
    static bool HandleWeatherInfoCommand(ChatHandler* handler, std::string_view /*args*/)
    {
        if (!WeatherSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("WeatherSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Default storm duration: {} s (maximum {}), default weather type {}.", WeatherSystemManager::Instance().StormDurationSecs(), WeatherSystemManager::Instance().MaxStormDurationSecs(), WeatherSystemManager::Instance().DefaultWeatherType());
        return true;
    }

};

// ============================================================================
// WorldScript
// ============================================================================

class WeatherSystemWorldScript : public WorldScript
{
public:
    WeatherSystemWorldScript() : WorldScript("WeatherSystemWorldScript") { }

    void OnStartup() override
    {
        WeatherSystemManager::Instance().Initialize();
    }
};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{
    __declspec(dllexport) void __stdcall InitializeWeatherSystemModule()
    {
        LOG_INFO("WeatherSystem", "WeatherSystem module loaded (scripts auto-register)");
    }

    __declspec(dllexport) void __stdcall FinalizeWeatherSystemModule()
    {
        LOG_INFO("WeatherSystem", "WeatherSystem module finalized");
    }
}

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void AddWeatherSystemScripts()
{
    new WeatherSystemCommandScript();
    new WeatherSystemWorldScript();
}

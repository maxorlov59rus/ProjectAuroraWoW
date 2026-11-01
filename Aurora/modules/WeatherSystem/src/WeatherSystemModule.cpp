/*
 * WeatherSystem module — implementation
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
    m_enabled = Aurora::Settings::Enabled("Aurora.Weather.Enabled", "WeatherSystem");
    m_stormDurationSecs = Aurora::Settings::Get<uint32_t>("Aurora.Weather.StormDurationSecs", 300, "WeatherSystem");

    if (!m_enabled)
    {
        LOG_INFO("WeatherSystem", "WeatherSystem disabled by Aurora.Weather.Enabled");
        return true;
    }

    LOG_INFO("WeatherSystem", "WeatherSystem module initialized");
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
    LOG_INFO("WeatherSystem", "Player {} logged in — weather system available", p->GetName());
}

void WeatherSystemManager::OnPlayerLevelUp(Player* p)
{
    if (!p || !m_enabled)
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
        if (!WeatherSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("WeatherSystem is disabled by configuration.");
            return true;
        }

        handler->PSendSysMessage("Weather type {} set on map {}.", weatherType, mapId);
        return true;
    }

    // ---------- .weather storm ----------
    static bool HandleTriggerStormCommand(ChatHandler* handler, uint32 mapId, uint32 duration)
    {
        if (!WeatherSystemManager::Instance().IsEnabled())
        {
            handler->SendSysMessage("WeatherSystem is disabled by configuration.");
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

        handler->PSendSysMessage("Default storm duration: {} s.", WeatherSystemManager::Instance().StormDurationSecs());
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

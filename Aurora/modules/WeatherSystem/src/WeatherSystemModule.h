#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class WeatherSystemManager {
public:
    static WeatherSystemManager& Instance() { static WeatherSystemManager inst; return inst; }
    bool Initialize() { LOG_INFO("WeatherSystem", "Weather system initialized"); return true; }
    void OnPlayerLogin(Player* p) { LOG_INFO("WeatherSystem", "Player %s logged in - weather system available", p->GetName().c_str()); }
    void OnPlayerLevelUp(Player* p) { LOG_INFO("WeatherSystem", "Player %s leveled up", p->GetName().c_str()); }
    void SetWeather(uint32_t mapId, uint32_t weatherType) { LOG_INFO("WeatherSystem", "Set weather type %u on map %u", weatherType, mapId); }
    void TriggerStorm(uint32_t mapId, uint32_t duration) { LOG_INFO("WeatherSystem", "Triggered storm on map %u for %u seconds", mapId, duration); }
};

void RegisterWeatherSystemCommands();
bool HandleSetWeatherCommand(ChatHandler* h, const std::string& a);
bool HandleTriggerStormCommand(ChatHandler* h, const std::string& a);
bool HandleWeatherInfoCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializeWeatherSystemModule();
    __declspec(dllexport) void __stdcall FinalizeWeatherSystemModule();
}
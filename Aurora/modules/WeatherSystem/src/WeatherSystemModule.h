#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class WeatherSystemManager {
public:
    WeatherSystemManager();
    ~WeatherSystemManager();
    static WeatherSystemManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    void SetWeather(uint32_t mapId, uint32_t weatherType);
    void TriggerStorm(uint32_t mapId, uint32_t duration);
    bool IsEnabled() const;
    uint32_t StormDurationSecs() const { return m_stormDurationSecs; }

private:
    bool m_enabled = true;
    uint32_t m_stormDurationSecs = {};
};


extern "C" {
    __declspec(dllexport) void __stdcall InitializeWeatherSystemModule();
    __declspec(dllexport) void __stdcall FinalizeWeatherSystemModule();
}

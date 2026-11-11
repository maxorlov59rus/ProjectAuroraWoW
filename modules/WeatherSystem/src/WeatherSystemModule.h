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
    bool IsEnabled() const;
    uint32_t StormDurationSecs() const { return m_stormDurationSecs; }
    uint32_t MaxStormDurationSecs() const { return m_maxStormDurationSecs; }
    uint32_t DefaultWeatherType() const { return m_defaultWeatherType; }
    uint32_t MaxWeatherType() const { return m_maxWeatherType; }
    bool IsValidWeatherType(uint32_t weatherType) const;
    void SetWeather(uint32_t mapId, uint32_t weatherType);
    void TriggerStorm(uint32_t mapId, uint32_t duration);

private:
    bool m_enabled = true;
    uint32_t m_stormDurationSecs = {};
    uint32_t m_maxStormDurationSecs = {};
    uint32_t m_defaultWeatherType = {};
    uint32_t m_maxWeatherType = {};
};
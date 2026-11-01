/*
 * TeleportationModule — модуль телепортации для AzerothCore (Wotlk)
 *
 * Предоставляет систему телепортации игроков в различные локации.
 */

#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
class WorldSession;
class Player;
class ChatHandler;

// Структура точки телепортации
struct TeleportLocation
{
    std::string name;
    float x, y, z;
    uint32_t mapId;
    std::string description;
};

// Глобальный менеджер телепортации
class TeleportationManager
{
public:
    static TeleportationManager& Instance()
    {
        static TeleportationManager inst;
        return inst;
    }

    // Инициализация менеджера
    bool Initialize();

    // Добавить точку телепортации
    bool AddLocation(const std::string& name, float x, float y, float z, uint32_t mapId, const std::string& description = "");

    // Удалить точку телепортации
    bool RemoveLocation(const std::string& name);

    // Телепортировать игрока в локацию
    bool TeleportPlayer(Player* pPlayer, const std::string& locationName);

    // Получить список всех локаций
    std::vector<TeleportLocation> GetLocations() const;

    // Проверить, существует ли локация
    bool LocationExists(const std::string& name) const;

private:
    TeleportationManager();
    ~TeleportationManager();

    std::map<std::string, TeleportLocation> m_locations;
    mutable std::mutex m_mutex;
};

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeTeleportationModule();
    __declspec(dllexport) void __stdcall FinalizeTeleportationModule();
}
/*
 * ClasslessSystemModule — модуль "безклассовой" системы для AzerothCore (Wotlk)
 *
 * Суть: игроки не привязаны кFixed-класу (WARRIOR, MAGE и т.д.).
 * Их роль определяется распределением очков навыков (skill points),
 * а также настройками из конфига и Lua-скриптов.
 *
 * Предоставляет:
 *   - Команда .classless <name> — создать безклассового игрока/бота
 *   - Команда .setskills <points> — выдать очки скиллов
 *   - Интеграция с AuroraSkills (усь-used skill system)
 *   - Хуки для модификации AI (OnUpdate, OnSpellCast) без класса
 */

#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
class WorldSession;
class Player;
class ChatHandler;

// ---------- Classless player info ----------
struct ClasslessInfo
{
    std::string name;
    uint32_t level = 1;
    // Skill points the player can spend (linked to AuroraSkills)
    uint32_t skillPoints = 0;
    // Map of spent points per school/lineage (simplified)
    std::map<std::string, uint32_t> spentSkills;
    // Optional Lua script name for AI
    std::string luaScript;
};

// ---------- Global manager ----------
class ClasslessManager
{
public:
    ClasslessManager();
    ~ClasslessManager();
    static ClasslessManager& Instance()
    {
        static ClasslessManager inst;
        return inst;
    }

    // Create a new classless player. Guid can be provided or auto-generated.
    // If playerGuid == 0 a new player is created in the world.
    bool CreateClasslessPlayer(const std::string& name,
                              uint32_t level,
                              uint64_t playerGuid = 0,
                              const std::string& luaScript = "");

    // Delete a classless player by GUID.
    bool DeleteClasslessPlayer(uint64_t guid);

    // Get info about a classless player.
    bool GetClasslessInfo(uint64_t guid, ClasslessInfo& out);

    // List all classless players.
    std::vector<ClasslessInfo> ListClassless() const;

    // Set skill points for a player.
    bool SetSkillPoints(uint64_t guid, uint32_t points);

    // Add skill points (e.g., from level-up).
    bool AddSkillPoints(uint64_t guid, uint32_t points);

    bool Initialize();
    bool IsEnabled() const { return m_enabled; }
    uint32_t PointsPerLevel() const { return m_pointsPerLevel; }

private:
    bool m_enabled = true;
    uint32_t m_pointsPerLevel = 2;

    std::map<uint64_t, ClasslessInfo> m_players;
    uint64_t m_nextGuid = 1;
    mutable std::mutex m_mutex;
};

// ---------- Helper: map class name to AzerothCore class mask ----------
uint32_t GetClassMaskForName(const std::string& name);

// Helper: check if a player is classless.
bool IsClassless(Player* pPlayer);

// ---------- Module registration ----------
extern "C"
{
    __declspec(dllexport) void __stdcall InitializeClasslessSystemModule();
    __declspec(dllexport) void __stdcall FinalizeClasslessSystemModule();
}
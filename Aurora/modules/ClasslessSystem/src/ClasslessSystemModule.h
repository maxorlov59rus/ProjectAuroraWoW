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

#include <string>
#include <map>
#include <mutex>

#include <AuroraPCH.h>

// Forward declarations for AzerothCore APIs
struct WorldSession;
struct Player;
struct ChatHandler;

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

private:
    std::map<uint64_t, ClasslessInfo> m_players;
    uint64_t m_nextGuid = 1;
    std::mutex m_mutex;
};

// ---------- Command declarations (will be defined in .cpp) ----------
void RegisterClasslessCommands();
bool HandleClasslessCommand(ChatHandler* handler, const std::string& args);
bool HandleSetSkillsCommand(ChatHandler* handler, const std::string& args);
bool HandleListClasslessCommand(ChatHandler* handler, const std::string& args);

// ---------- Helper: map class name to AzerothCore class mask ----------
uint32_t GetClassMaskForName(const std::string& name);

// Helper: check if a player is classless.
bool IsClassless(Player* pPlayer);

// Hook: called when a player logs in, apply classless modifications.
void OnPlayerLogin(Player* pPlayer);

// Hook: called on level-up, redistribute skill points if needed.
void OnPlayerLevelUp(Player* pPlayer);

// -------------------------------------------------------------------------
// Inline definitions (kept minimal to avoid ODR issues in header)
// -------------------------------------------------------------------------

inline uint32_t GetClassMaskForName(const std::string& name)
{
    // Simple mapping; AzerothCore uses bitmask constants PLAYER_CLASS_*.
    if (name == "WARRIOR")  return 0x00000001; // CLASS_WARRIOR
    if (name == "MAGE")     return 0x00000002; // CLASS_MAGE
    if (name == "ROGUE")    return 0x00000004; // CLASS_ROGUE
    if (name == "DRUID")    return 0x00000008; // CLASS_DRUID
    if (name == "PALADIN")  return 0x00000010; // CLASS_PALADIN
    if (name == "PRIEST")   return 0x00000020; // CLASS_PRIEST
    if (name == "SHAMAN")   return 0x00000040; // CLASS_SHAMAN
    if (name == "WARLOCK")  return 0x00000080; // CLASS_WARLOCK
    if (name == "HUNTER")   return 0x00000100; // CLASS_HUNTER
    // "NONE" or empty -> classless (bit 0 reserved for flag)
    return 0x00000001; // fallback
}

inline bool IsClassless(Player* pPlayer)
{
    if (!pPlayer) return false;
    // Check if the class mask equals the "none" flag (we'll use bit 31 for classless)
    uint32_t mask = pPlayer->getClassMask(); // AzerothCore method
    return (mask & 0x80000000) != 0; // we reserve MSB as classless flag
}

// -------------------------------------------------------------------------
// End of header
// -------------------------------------------------------------------------
#endif // CLASSLESS_SYSTEM_MODULE_H
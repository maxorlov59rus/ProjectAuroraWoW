/*
 * ClasslessSystemModule - "classless" system for AzerothCore (WotLK)
 *
 * Idea: players are not bound to a fixed class (WARRIOR, MAGE, ...). Their
 * role comes from distributed skill points, module settings and Lua scripts.
 *
 * Provides:
 *   - .classless <name>  - register a classless character/bot
 *   - .setskills <points> - grant skill points
 *   - integration with AuroraSkills (the shared skill system)
 *   - hooks to modify AI (OnUpdate, OnSpellCast) without a class
 */

#pragma once

#include "Define.h"
#include <string>
#include <map>
#include <mutex>
#include <vector>

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
    // If playerGuid == 0 a new player is created in the world. When outGuid is
    // given it receives the guid the character was registered under, which lets
    // callers report the stored (capped) values instead of the requested ones.
    bool CreateClasslessPlayer(const std::string& name,
                              uint32_t level,
                              uint64_t playerGuid = 0,
                              const std::string& luaScript = "",
                              uint64_t* outGuid = nullptr);

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
    uint32_t MaxLevel() const { return m_maxLevel; }
    // 0 means "no cap"; otherwise the ceiling for accumulated skill points.
    uint32_t MaxSkillPoints() const { return m_maxSkillPoints; }
    std::string const& DefaultScript() const { return m_defaultScript; }

private:
    bool m_enabled = true;
    uint32_t m_pointsPerLevel = 2;
    uint32_t m_maxLevel = 80;
    uint32_t m_maxSkillPoints = 0;
    std::string m_defaultScript;

    std::map<uint64_t, ClasslessInfo> m_players;
    uint64_t m_nextGuid = 1;
    mutable std::mutex m_mutex;
};
// ---------- Helper: map class name to AzerothCore class mask ----------
uint32_t GetClassMaskForName(const std::string& name);

// Helper: check if a player is classless.
bool IsClassless(Player* pPlayer);

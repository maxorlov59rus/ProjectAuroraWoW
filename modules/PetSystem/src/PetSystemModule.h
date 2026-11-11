#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class PetSystemManager {
public:
    PetSystemManager();
    ~PetSystemManager();
    static PetSystemManager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
    uint32_t DefaultPetLevel() const { return m_defaultPetLevel; }
    uint32_t MaxPetLevel() const { return m_maxPetLevel; }
    void SpawnPet(Player* p, uint32_t entry);
    void RemovePet(Player* p);
    void SetPetLevel(Player* p, uint32_t level);
    void SetPetStats(Player* p, uint32_t health, uint32_t mana);

private:
    bool m_enabled = true;
    uint32_t m_defaultPetLevel = {};
    uint32_t m_maxPetLevel = {};
};
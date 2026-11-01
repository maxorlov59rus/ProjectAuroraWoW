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
    void SpawnPet(Player* p, uint32_t entry);
    void RemovePet(Player* p);
    void SetPetLevel(Player* p, uint32_t level);
    void SetPetStats(Player* p, uint32_t health, uint32_t mana);
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
};

extern "C" {
    __declspec(dllexport) void __stdcall InitializePetSystemModule();
    __declspec(dllexport) void __stdcall FinalizePetSystemModule();
}

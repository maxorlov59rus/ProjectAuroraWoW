#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

struct WorldSession; struct Player; struct ChatHandler;

class PetSystemManager {
public:
    static PetSystemManager& Instance() { static PetSystemManager inst; return inst; }
    bool Initialize() { LOG_INFO("PetSystem", "Pet system initialized"); return true; }
    void SpawnPet(Player* p, uint32_t entry) { LOG_INFO("PetSystem", "Spawned pet entry %u for player", entry); }
    void RemovePet(Player* p) { LOG_INFO("PetSystem", "Removed pet for player"); }
    void SetPetLevel(Player* p, uint32_t level) { LOG_INFO("PetSystem", "Set pet level to %u", level); }
    void SetPetStats(Player* p, uint32_t health, uint32_t mana) { LOG_INFO("PetSystem", "Set pet stats %u/%u", health, mana); }
    void OnPlayerLogin(Player* p) { LOG_INFO("PetSystem", "Player %s logged in - pet system available", p->GetName().c_str()); }
};

void RegisterPetSystemCommands();
bool HandleSpawnPetCommand(ChatHandler* h, const std::string& a);
bool HandleRemovePetCommand(ChatHandler* h, const std::string& a);
bool HandleSetPetLevelCommand(ChatHandler* h, const std::string& a);
bool HandleSetPetStatsCommand(ChatHandler* h, const std::string& a);

extern "C" {
    __declspec(dllexport) void __stdcall InitializePetSystemModule();
    __declspec(dllexport) void __stdcall FinalizePetSystemModule();
}
#include "PetSystemModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>

PetSystemManager::PetSystemManager() {}
PetSystemManager::~PetSystemManager() {}

bool PetSystemManager::SpawnPet(Player* p, uint32_t entry) {
    LOG_INFO("PetSystem", "Spawned pet entry %u for player %s", entry, p->GetName().c_str());
    return true;
}

bool PetSystemManager::RemovePet(Player* p) {
    LOG_INFO("PetSystem", "Removed pet for player %s", p->GetName().c_str());
    return true;
}

void PetSystemManager::SetPetLevel(Player* p, uint32_t level) {
    LOG_INFO("PetSystem", "Set pet level to %u for player %s", level, p->GetName().c_str());
}

void PetSystemManager::SetPetStats(Player* p, uint32_t health, uint32_t mana) {
    LOG_INFO("PetSystem", "Set pet stats %u/%u for player %s", health, mana, p->GetName().c_str());
}

void PetSystemManager::OnPlayerLogin(Player* p) {
    LOG_INFO("PetSystem", "Player %s logged in - pet system available", p->GetName().c_str());
}

void RegisterPetSystemCommands() {
    LOG_INFO("PetSystem", "Pet system commands registered: .spawnpet, .removepet, .setpetlevel, .setpetstats");
}

bool HandleSpawnPetCommand(ChatHandler* h, const std::string& a) {
    if (a.empty()) { h->SendSysMessage("Использование: .spawnpet <entry>"); return true; }
    uint32_t entry = uint32_t(std::stoul(a));
    h->SendSysMessage("Спавнен пет entry %u (заглушка)", entry);
    return true;
}

bool HandleRemovePetCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Питомец удален (заглушка)");
    return true;
}

bool HandleSetPetLevelCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Уровень питомца установлен (заглушка)");
    return true;
}

bool HandleSetPetStatsCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Статы питомца установлены (заглушка)");
    return true;
}

extern "C" {
    __declspec(dllexport) void __stdcall InitializePetSystemModule() {
        PetSystemManager::Instance().Initialize();
        RegisterPetSystemCommands();
        LOG_INFO("PetSystem", "PetSystem module initialized");
    }
    __declspec(dllexport) void __stdcall FinalizePetSystemModule() {
        LOG_INFO("PetSystem", "PetSystem module finalized");
    }
}
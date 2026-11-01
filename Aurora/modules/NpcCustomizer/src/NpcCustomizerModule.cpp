#include "NpcCustomizerModule.h"
#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <ScriptMgr.h>
#include <Player.h>
#include <Chat.h>

NpcCustomizerManager::NpcCustomizerManager() {}
NpcCustomizerManager::~NpcCustomizerManager() {}

bool NpcCustomizerManager::SpawnNpc(uint32_t npcEntry, float x, float y, float z, uint32_t mapId, const std::string& name) {
    LOG_INFO("NpcCustomizer", "Spawned NPC entry %u at (%.2f, %.2f, %.2f) map %u", npcEntry, x, y, z, mapId);
    return true;
}

bool NpcCustomizerManager::RemoveNpc(uint64_t guid) {
    LOG_INFO("NpcCustomizer", "Removed NPC GUID: %llu", (unsigned long long)guid);
    return true;
}

void NpcCustomizerManager::SetNpcName(uint64_t guid, const std::string& name) { LOG_INFO("NpcCustomizer", "Set NPC %llu name to %s", (unsigned long long)guid, name.c_str()); }
void NpcCustomizerManager::SetNpcLevel(uint64_t guid, uint32_t level) { LOG_INFO("NpcCustomizer", "Set NPC %llu level to %u", (unsigned long long)guid, level); }
void NpcCustomizerManager::SetNpcHealth(uint64_t guid, uint32_t health, uint32_t maxHealth) { LOG_INFO("NpcCustomizer", "Set NPC %llu health to %u/%u", (unsigned long long)guid, health, maxHealth); }
void NpcCustomizerManager::SetNpcFaction(uint64_t guid, uint32_t faction) { LOG_INFO("NpcCustomizer", "Set NPC %llu faction to %u", (unsigned long long)guid, faction); }
void NpcCustomizerManager::SetNpcFlags(uint64_t guid, uint32_t flags) { LOG_INFO("NpcCustomizer", "Set NPC %llu flags to %u", (unsigned long long)guid, flags); }
std::vector<uint64_t> NpcCustomizerManager::GetSpawnedNpcs() const { return std::vector<uint64_t>(); }

void RegisterNpcCustomizerCommands() { LOG_INFO("NpcCustomizer", "NPC Customizer commands registered"); }

bool HandleSpawnNpcCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Spawn NPC command received (placeholder)");
    return true;
}

bool HandleRemoveNpcCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Remove NPC command received (placeholder)");
    return true;
}

bool HandleSetNpcNameCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Set NPC name command received (placeholder)");
    return true;
}

bool HandleSetNpcLevelCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Set NPC level command received (placeholder)");
    return true;
}

bool HandleSetNpcHealthCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Set NPC health command received (placeholder)");
    return true;
}

bool HandleSetNpcFactionCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Set NPC faction command received (placeholder)");
    return true;
}

bool HandleSetNpcFlagsCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("Set NPC flags command received (placeholder)");
    return true;
}

bool HandleListNpcsCommand(ChatHandler* h, const std::string& a) {
    h->SendSysMessage("List NPCs command received (placeholder)");
    return true;
}

void OnPlayerLogin(Player* p) { LOG_INFO("NpcCustomizer", "Player %s logged in", p->GetName().c_str()); }
void OnPlayerLevelUp(Player* p) { LOG_INFO("NpcCustomizer", "Player %s leveled up", p->GetName().c_str()); }

extern "C" {
    __declspec(dllexport) void __stdcall InitializeNpcCustomizerModule() {
        RegisterNpcCustomizerCommands();
        LOG_INFO("NpcCustomizer", "NpcCustomizer module initialized");
    }
    __declspec(dllexport) void __stdcall FinalizeNpcCustomizerModule() {
        LOG_INFO("NpcCustomizer", "NpcCustomizer module finalized");
    }
}
// ---------- Command registrations ----------

void RegisterNpcCustomizerCommands()
{
    LOG_INFO("NpcCustomizer", "NPC Customizer commands registered: .spawnnpc, .removenpc, .setnpcname, .setnpclevel, .setnpchp, .setnpcfaction, .setnpcflags, .listnpcs");
}

bool HandleSpawnNpcCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 4) {
        handler->SendSysMessage("Использование: .spawnnpc <entry> <x> <y> <z> [mapId] [name]");
        return true;
    }

    uint32_t entry = uint32_t(std::stoul(tokens[0]));
    float x = std::stof(tokens[1]);
    float y = std::stof(tokens[1]);
    float z = std::stof(tokens[1]);
    uint32_t mapId = (tokens.size() > 4) ? uint32_t(std::stoul(tokens[4])) : 0;
    std::string name = (tokens.size() > 5) ? tokens[5] : "";

    if (NpcCustomizerManager::Instance().SpawnNpc(entry, x, y, z, mapId, name)) {
        handler->SendSysMessage("NPC entry %u spawned at (%.2f, %.2f, %.2f) map %u", entry, x, y, z, mapId);
    } else {
        handler->SendSysMessage("Не удалось спавн NPC entry %u", entry);
    }
    return true;
}

bool HandleRemoveNpcCommand(ChatHandler* handler, const std::string& args)
{
    if (args.empty()) {
        handler->SendSysMessage("Использование: .removenpc <guid>");
        return true;
    }

    uint64_t guid = 0;
    try { guid = std::stoull(args); } catch (...) { guid = 0; }

    if (NpcCustomizerManager::Instance().RemoveNpc(guid)) {
        handler->SendSysMessage("NPC GUID %llu удален.", (unsigned long long)guid);
    } else {
        handler->SendSysMessage("NPC GUID %llu не найден.", (unsigned long long)guid);
    }
    return true;
}

bool HandleSetNpcNameCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .setnpcname <guid> <name>");
        return true;
    }

    uint64_t guid = 0;
    try { guid = std::stoull(tokens[0]); } catch (...) { guid = 0; }
    std::string name = tokens[1];

    NpcCustomizerManager::Instance().SetNpcName(guid, name);
    handler->SendSysMessage("NPC %llu имя установлено на %s", (unsigned long long)guid, name.c_str());
    return true;
}

bool HandleSetNpcLevelCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .setnpclevel <guid> <level>");
        return true;
    }

    uint64_t guid = 0;
    uint32_t level = 0;
    try { guid = std::stoull(tokens[0]); level = uint32_t(std::stoul(tokens[1])); } catch (...) { guid = 0; }

    NpcCustomizerManager::Instance().SetNpcLevel(guid, level);
    handler->SendSysMessage("NPC %llu уровень установлен на %u", (unsigned long long)guid, level);
    return true;
}

bool HandleSetNpcHealthCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 3) {
        handler->SendSysMessage("Использование: .setnpchp <guid> <health> <maxHealth>");
        return true;
    }

    uint64_t guid = 0;
    uint32_t health = 0, maxHealth = 0;
    try { guid = std::stoull(tokens[0]); health = uint32_t(std::stoul(tokens[1])); maxHealth = uint32_t(std::stoul(tokens[2])); } catch (...) { guid = 0; }

    NpcCustomizerManager::Instance().SetNpcHealth(guid, health, maxHealth);
    handler->SendSysMessage("NPC %llu здоровье установлено на %u/%u", (unsigned long long)guid, health, maxHealth);
    return true;
}

bool HandleSetNpcFactionCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .setnpcfaction <guid> <faction>");
        return true;
    }

    uint64_t guid = 0;
    uint32_t faction = 0;
    try { guid = std::stoull(tokens[0]); faction = uint32_t(std::stoul(tokens[1])); } catch (...) { guid = 0; }

    NpcCustomizerManager::Instance().SetNpcFaction(guid, faction);
    handler->SendSysMessage("NPC %llu фракция установлена на %u", (unsigned long long)guid, faction);
    return true;
}

bool HandleSetNpcFlagsCommand(ChatHandler* handler, const std::string& args)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    std::string rem = args;
    while ((pos = rem.find(' ')) != std::string::npos) {
        token = rem.substr(0, pos);
        tokens.push_back(token);
        rem.erase(0, pos + 1);
    }
    tokens.push_back(rem);

    if (tokens.size() < 2) {
        handler->SendSysMessage("Использование: .setnpcflags <guid> <flags>");
        return true;
    }

    uint64_t guid = 0;
    uint32_t flags = 0;
    try { guid = std::stoull(tokens[0]); flags = uint32_t(std::stoul(tokens[1])); } catch (...) { guid = 0; }

    NpcCustomizerManager::Instance().SetNpcFlags(guid, flags);
    handler->SendSysMessage("NPC %llu флаги установлены на %u", (unsigned long long)guid, flags);
    return true;
}

bool HandleListNpcsCommand(ChatHandler* handler, const std::string& args)
{
    auto npcs = NpcCustomizerManager::Instance().GetSpawnedNpcs();
    if (npcs.empty()) {
        handler->SendSysMessage("Спавненых NPC нет.");
        return true;
    }

    handler->SendSysMessage("Спавненых NPC (всего: %d):", (int)npcs.size());
    for (const auto& guid : npcs) {
        handler->SendSysMessage("- GUID: %llu", (unsigned long long)guid);
    }
    return true;
}

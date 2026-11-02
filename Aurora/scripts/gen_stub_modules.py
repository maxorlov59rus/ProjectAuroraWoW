"""
Generator for the small Aurora stub modules.

Every module listed below has the same shape - a manager holding its settings,
a handful of GM commands and a config-driven enable switch - so both the header
and the implementation are generated from one spec instead of hand-writing eight
near-identical pairs of files.

The spec is the single source of truth for these files: rerunning this script
overwrites modules/<Name>/src/<Name>Module.{h,cpp}, so real changes to those two
files belong in the spec below, not in the generated output.

Rule for the spec: every option listed here is read in Initialize() AND used by
the manager or a command. An option that is only logged is dead configuration,
which is exactly what the module configs are meant to avoid.

Run:  python scripts/gen_stub_modules.py
Then: python scripts/migrate_to_ac.py
"""
from pathlib import Path

MODULES = Path(__file__).resolve().parent.parent / "modules"

# the separator the generated blocks are joined with
NL = chr(10)

HEADER_FILE = """#pragma once
#include "Define.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <AuroraPCH.h>

class WorldSession; class Player; class ChatHandler;

class {cls}Manager {{
public:
    {cls}Manager();
    ~{cls}Manager();
    static {cls}Manager& Instance();
    bool Initialize();
    void OnPlayerLogin(Player* p);
    void OnPlayerLevelUp(Player* p);
    bool IsEnabled() const;
{public_extra}
private:
    bool m_enabled = true;
{fields}}};


extern "C" {{
    __declspec(dllexport) void __stdcall Initialize{name}Module();
    __declspec(dllexport) void __stdcall Finalize{name}Module();
}}
"""

TEMPLATE = """/*
 * {name} module - implementation
 *
 * {blurb}
 *
 * This module registers two AzerothCore scripts:
 *   - {cls}CommandScript : CommandScript (chat commands)
 *   - {cls}WorldScript    : WorldScript  (reads the module options at startup)
 *
 * Settings live in configs/modules/{conf} (see {name}/conf/{conf}.dist). They
 * are read in WorldScript::OnStartup, not in a static initializer, because the
 * core only loads the module config files after its own one.
 */

#include "{name}Module.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>
#include <AuroraModuleSettings.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
#include <WorldScript.h>
#include <Player.h>
#include <Chat.h>
#include <ChatCommandTags.h>
#include <RBAC.h>

// ============================================================================
// {cls}Manager - implementation
// ============================================================================

{cls}Manager::{cls}Manager() = default;

{cls}Manager::~{cls}Manager() = default;

{cls}Manager& {cls}Manager::Instance()
{{
    static {cls}Manager inst;
    return inst;
}}

bool {cls}Manager::Initialize()
{{
    m_enabled = Aurora::Settings::Enabled("{cfg}.Enabled", "{name}");
{load_settings}
    if (!m_enabled)
    {{
        LOG_INFO("@NAME@", "{name} disabled by {cfg}.Enabled");
        return true;
    }}

{init_log}
    return true;
}}

bool {cls}Manager::IsEnabled() const
{{
    return m_enabled;
}}

void {cls}Manager::OnPlayerLogin(Player* p)
{{
    if (!p || !m_enabled)
        return;
    LOG_INFO("@NAME@", "Player {{}} logged in - {human} available", p->GetName());
}}

void {cls}Manager::OnPlayerLevelUp(Player* p)
{{
    if (!p || !m_enabled)
        return;
    LOG_INFO("@NAME@", "Player {{}} leveled up", p->GetName());
}}
{extra_impl}
// ============================================================================
// CommandScript
// ============================================================================

class {cls}CommandScript : public CommandScript
{{
public:
    {cls}CommandScript() : CommandScript("{cls}CommandScript") {{ }}

    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override
    {{
        using namespace Acore::ChatCommands;

        static ChatCommandTable {prefix}Table =
        {{
{table}
        }};

        static ChatCommandTable commandTable =
        {{
            {{ "{prefix}", {prefix}Table }}
        }};

        return commandTable;
    }}

private:
{handlers}}};

// ============================================================================
// WorldScript
// ============================================================================

class {cls}WorldScript : public WorldScript
{{
public:
    {cls}WorldScript() : WorldScript("{cls}WorldScript") {{ }}

    void OnStartup() override
    {{
        {cls}Manager::Instance().Initialize();
    }}
}};

// ============================================================================
// Module entry points (for the old .dll loader)
// ============================================================================

extern "C"
{{
    __declspec(dllexport) void __stdcall Initialize{name}Module()
    {{
        LOG_INFO("@NAME@", "{name} module loaded (scripts auto-register)");
    }}

    __declspec(dllexport) void __stdcall Finalize{name}Module()
    {{
        LOG_INFO("@NAME@", "{name} module finalized");
    }}
}}

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this from sScriptMgr->Initialize(),
// once worldserver.conf and configs/modules/*.conf are loaded and the script
// registries are usable. Creating the scripts here, rather than through
// file-scope objects, keeps registration out of static initialization and is
// what lets WorldScript::OnStartup read the module options.
// ============================================================================

void {loader}()
{{
    new {cls}CommandScript();
    new {cls}WorldScript();
}}
"""

HANDLER = """    // ---------- .{prefix} {sub} ----------
    static bool {handler}(ChatHandler* handler{argdecl})
    {{
{guard}{extra_guard}{pre}        {body}
        return true;
    }}

"""

GUARD = """        if (!{cls}Manager::Instance().IsEnabled())
        {{
            handler->SendSysMessage("{name} is disabled by configuration.");
            return true;
        }}

"""

# A command is a dict:
#   sub        subcommand name
#   handler    handler function name
#   args       extra parameters of the handler, starting with ", "
#   msg        message; "{}" placeholders are filled from fmt
#   fmt        format arguments for msg
#   pre        code run before the message (validations, calculations)
#   guard      extra early-exit code, e.g. a permission check
CMD = dict


def m(cls):  # shorthand used in the specs below
    return f"{cls}Manager::Instance()"


SPECS = [
    {
        "name": "AdminTools", "cls": "AdminTools", "prefix": "admin",
        "human": "admin tools", "cfg": "Aurora.AdminTools",
        "conf": "aurora_admintools.conf",
        "blurb": "GM helpers: admin panel, server info and an umbrella command.",
        "fields": ["    bool m_requireSession = {};"],
        "public_extra": ["    bool RequireSession() const { return m_requireSession; }"],
        "settings":
            '    m_requireSession = Aurora::Settings::Enabled("{cfg}.RequireSession", "{name}", true);\n',
        "init_log":
            '    LOG_INFO("@NAME@", "AdminTools module initialized (console access: {})",\n'
            '        m_requireSession ? "denied" : "allowed");',
        "commands": [
            CMD(sub="adminpanel", handler="HandleAdminPanelCommand",
                msg="Admin panel is {}.",
                fmt=f'{m("AdminTools")}.RequireSession() ? "restricted to in-game GMs" : "also available from the console"',
                extra_guard='        if (handler->IsConsole() && {mgr}.RequireSession())\n'
                            '        {\n'
                            '            handler->SendSysMessage("The admin panel needs an in-game GM session.");\n'
                            '            return true;\n'
                            '        }\n\n'),
            CMD(sub="serverinfo", handler="HandleServerInfoCommand",
                msg="Server info command received.",
                extra_guard='        if (handler->IsConsole() && {mgr}.RequireSession())\n'
                            '        {\n'
                            '            handler->SendSysMessage("The admin commands need an in-game GM session.");\n'
                            '            return true;\n'
                            '        }\n\n'),
            CMD(sub="help", handler="HandleAdminCommand",
                msg="Commands: .admin adminpanel, .admin serverinfo, .admin help."),
        ],
    },
    {
        "name": "ArenaSystem", "cls": "ArenaSystem", "prefix": "arena",
        "human": "arena system", "cfg": "Aurora.Arena",
        "conf": "aurora_arena.conf",
        "blurb": "Arena team helpers: queueing, ranks and per-team statistics.",
        "fields": ["    uint32_t m_ratedMinLevel = {};"],
        "public_extra": ["    uint32_t RatedMinLevel() const { return m_ratedMinLevel; }"],
        "settings":
            '    m_ratedMinLevel = Aurora::Settings::Bounded<uint32_t>("{cfg}.RatedMinimumLevel", 80, 1, 80, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "ArenaSystem module initialized (rated arena from level {})", m_ratedMinLevel);',
        "commands": [
            CMD(sub="queue", handler="HandleArenaQueueCommand",
                # The level requirement is what makes RatedMinimumLevel a setting
                # instead of a number that only shows up in .arena stats.
                pre='        Player* p = handler->GetPlayer();\n'
                    '        if (p && p->GetLevel() < ArenaSystemManager::Instance().RatedMinLevel())\n'
                    '        {\n'
                    '            handler->PSendSysMessage("Rated arena queue requires level {} (you are {}).",\n'
                    '                ArenaSystemManager::Instance().RatedMinLevel(), p->GetLevel());\n'
                    '            return true;\n'
                    '        }\n\n'
                    '        if (!p)\n'
                    '        {\n'
                    '            handler->SendSysMessage("The arena queue command needs an in-game character.");\n'
                    '            return true;\n'
                    '        }\n\n',
                msg="Arena queue request accepted for {}."),
            CMD(sub="rank", handler="HandleArenaRankCommand",
                msg="Arena rank command received."),
            CMD(sub="stats", handler="HandleArenaStatsCommand",
                msg="Rated arena requires level {}.",
                fmt=f'{m("ArenaSystem")}.RatedMinLevel()'),
        ],
    },
    {
        "name": "Economy", "cls": "Economy", "prefix": "economy",
        "human": "economy system", "cfg": "Aurora.Economy",
        "conf": "aurora_economy.conf",
        "blurb": "Custom economy hooks: auctions, currencies and exchanges.",
        "fields": ["    uint32_t m_auctionDepositPercent = {};"],
        "public_extra": ["    uint32_t AuctionDepositPercent() const { return m_auctionDepositPercent; }"],
        "settings":
            '    m_auctionDepositPercent = Aurora::Settings::Bounded<uint32_t>("{cfg}.AuctionDepositPercent", 5, 0, 100, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "Economy module initialized (auction deposit {} percent)", m_auctionDepositPercent);',
        "commands": [
            CMD(sub="auction", handler="HandleAuctionCommand",
                args=", uint32 bid",
                # The configured percent now decides the reported deposit.
                pre='        uint32 const deposit = static_cast<uint32_t>(\n'
                    '            static_cast<uint64_t>(bid) * EconomyManager::Instance().AuctionDepositPercent() / 100);\n\n',
                msg="Auction of {} copper needs a {} copper deposit ({} percent).",
                fmt=f"bid, deposit, {m('Economy')}.AuctionDepositPercent()"),
            CMD(sub="currency", handler="HandleCurrencyCommand",
                msg="Currency command received."),
            CMD(sub="exchange", handler="HandleExchangeCommand",
                msg="Exchange command received."),
        ],
    },
    {
        "name": "GuildEnhancements", "cls": "GuildEnhancements", "prefix": "guildx",
        "human": "guild enhancements", "cfg": "Aurora.GuildEnhancements",
        "conf": "aurora_guild.conf",
        "blurb": "Guild improvements: bank, ranks and extended info.",
        "fields": ["    uint32_t m_bankTabCost = {};"],
        "public_extra": ["    uint32_t BankTabCost() const { return m_bankTabCost; }"],
        "settings":
            '    m_bankTabCost = Aurora::Settings::Get<uint32_t>("{cfg}.BankTabCost", 100000, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "GuildEnhancements module initialized (bank tab {} copper)", m_bankTabCost);',
        "commands": [
            CMD(sub="bank", handler="HandleGuildBankCommand",
                # Copper is unreadable for players, so the configured price is
                # reported in the gold/silver/copper split the client uses.
                pre='        uint32 const cost = GuildEnhancementsManager::Instance().BankTabCost();\n\n',
                msg="A new guild bank tab costs {} gold, {} silver, {} copper.",
                fmt="cost / 10000, (cost / 100) % 100, cost % 100"),
            CMD(sub="rank", handler="HandleGuildRankCommand",
                msg="Guild rank command received."),
            CMD(sub="info", handler="HandleGuildInfoCommand",
                msg="Guild info command received."),
        ],
    },
    {
        "name": "LootSystem", "cls": "LootSystem", "prefix": "loot",
        "human": "loot system", "cfg": "Aurora.Loot",
        "conf": "aurora_loot.conf",
        "blurb": "Loot tuning: drop rates, loot inspection and reset.",
        "fields": ["    float m_dropRate = {};", "    float m_maxDropRate = {};"],
        "public_extra": [
            "    float DropRate() const { return m_dropRate; }",
            "    // Runtime override from .loot droprate; rejected above MaxDropRate().",
            "    bool SetDropRate(float rate);",
            "    float MaxDropRate() const { return m_maxDropRate; }",
        ],
        "settings":
            '    m_maxDropRate = Aurora::Settings::Get<float>("{cfg}.MaxDropRate", 10.0f, "{name}");\n'
            '    m_dropRate = Aurora::Settings::Bounded<float>("{cfg}.DropRate", 1.0f, 0.0f, m_maxDropRate, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "LootSystem module initialized (drop rate {}, max {})", m_dropRate, m_maxDropRate);',
        "extra_impl": """
bool LootSystemManager::SetDropRate(float rate)
{
    if (rate < 0.0f || rate > m_maxDropRate)
        return false;

    m_dropRate = rate;
    LOG_INFO("LootSystem", "Drop rate changed to {}", rate);
    return true;
}
""",
        "commands": [
            CMD(sub="droprate", handler="HandleSetDropRateCommand",
                args=", float rate",
                # Without the setter the command only echoed the argument and
                # the manager kept reporting the configured rate forever.
                pre='        if (!LootSystemManager::Instance().SetDropRate(rate))\n'
                    '        {\n'
                    '            handler->PSendSysMessage("Drop rate must be between 0 and {}.",\n'
                    '                LootSystemManager::Instance().MaxDropRate());\n'
                    '            return true;\n'
                    '        }\n\n',
                msg="Drop rate set to {}.",
                fmt=f'{m("LootSystem")}.DropRate()'),
            CMD(sub="info", handler="HandleLootInfoCommand",
                msg="Current drop rate: {} (maximum {}).",
                fmt=f'{m("LootSystem")}.DropRate(), {m("LootSystem")}.MaxDropRate()'),
            CMD(sub="reset", handler="HandleResetLootCommand",
                msg="Loot tables reset."),
        ],
    },
    {
        "name": "PetSystem", "cls": "PetSystem", "prefix": "pet",
        "human": "pet system", "cfg": "Aurora.Pets",
        "conf": "aurora_pets.conf",
        "blurb": "Custom pet handling: spawning, removal and stat overrides.",
        "fields": ["    uint32_t m_defaultPetLevel = {};", "    uint32_t m_maxPetLevel = {};"],
        "public_extra": [
            "    uint32_t DefaultPetLevel() const { return m_defaultPetLevel; }",
            "    uint32_t MaxPetLevel() const { return m_maxPetLevel; }",
            "    void SpawnPet(Player* p, uint32_t entry);",
            "    void RemovePet(Player* p);",
            "    void SetPetLevel(Player* p, uint32_t level);",
            "    void SetPetStats(Player* p, uint32_t health, uint32_t mana);",
        ],
        "settings":
            '    m_maxPetLevel = Aurora::Settings::Bounded<uint32_t>("{cfg}.MaxLevel", 80, 1, 80, "{name}");\n'
            '    m_defaultPetLevel = Aurora::Settings::Bounded<uint32_t>("{cfg}.DefaultLevel", 1, 1, m_maxPetLevel, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "PetSystem module initialized (default level {}, max {})",\n'
            '        m_defaultPetLevel, m_maxPetLevel);',
        "extra_impl": """
void PetSystemManager::SpawnPet(Player* p, uint32_t entry)
{
    if (!p)
        return;
    // TODO: create the pet creature through the map spawn helper.
    LOG_INFO("PetSystem", "Spawned pet entry {} for player {} at level {}",
        entry, p->GetName(), m_defaultPetLevel);
}

void PetSystemManager::RemovePet(Player* p)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Removed pet for player {}", p->GetName());
}

void PetSystemManager::SetPetLevel(Player* p, uint32_t level)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Set pet level to {} for player {}", level, p->GetName());
}

void PetSystemManager::SetPetStats(Player* p, uint32_t health, uint32_t mana)
{
    if (!p)
        return;
    LOG_INFO("PetSystem", "Set pet stats {}/{} for player {}", health, mana, p->GetName());
}
""",
        "commands": [
            CMD(sub="spawn", handler="HandleSpawnPetCommand",
                args=", uint32 entry",
                msg="Pet entry {} spawned at level {}.",
                fmt=f'entry, {m("PetSystem")}.DefaultPetLevel()'),
            CMD(sub="remove", handler="HandleRemovePetCommand",
                msg="Pet removed."),
            CMD(sub="level", handler="HandleSetPetLevelCommand",
                args=", uint32 level",
                pre='        if (!level || level > PetSystemManager::Instance().MaxPetLevel())\n'
                    '        {\n'
                    '            handler->PSendSysMessage("Pet level must be between 1 and {}.",\n'
                    '                PetSystemManager::Instance().MaxPetLevel());\n'
                    '            return true;\n'
                    '        }\n\n',
                msg="Pet level set to {}.",
                fmt="level"),
            CMD(sub="stats", handler="HandleSetPetStatsCommand",
                args=", uint32 health, uint32 mana",
                msg="Pet stats set to {}/{}.",
                fmt="health, mana"),
        ],
    },
    {
        "name": "QuestSystem", "cls": "QuestSystem", "prefix": "questx",
        "human": "quest system", "cfg": "Aurora.Quests",
        "conf": "aurora_quest.conf",
        "blurb": "Custom quest helpers: creation, inspection and completion.",
        "fields": [],
        "public_extra": [],
        "settings": "",
        "init_log": '    LOG_INFO("@NAME@", "QuestSystem module initialized");',
        "commands": [
            CMD(sub="create", handler="HandleCreateQuestCommand",
                msg="Create quest command received."),
            CMD(sub="info", handler="HandleQuestInfoCommand",
                msg="Quest info command received."),
            CMD(sub="complete", handler="HandleCompleteQuestCommand",
                msg="Complete quest command received."),
        ],
    },
    {
        "name": "WeatherSystem", "cls": "WeatherSystem", "prefix": "weather",
        "human": "weather system", "cfg": "Aurora.Weather",
        "conf": "aurora_weather.conf",
        "blurb": "Map weather control: set a weather type or trigger a timed storm.",
        "fields": [
            "    uint32_t m_stormDurationSecs = {};",
            "    uint32_t m_maxStormDurationSecs = {};",
            "    uint32_t m_defaultWeatherType = {};",
            "    uint32_t m_maxWeatherType = {};",
        ],
        "public_extra": [
            "    uint32_t StormDurationSecs() const { return m_stormDurationSecs; }",
            "    uint32_t MaxStormDurationSecs() const { return m_maxStormDurationSecs; }",
            "    uint32_t DefaultWeatherType() const { return m_defaultWeatherType; }",
            "    uint32_t MaxWeatherType() const { return m_maxWeatherType; }",
            "    bool IsValidWeatherType(uint32_t weatherType) const;",
            "    void SetWeather(uint32_t mapId, uint32_t weatherType);",
            "    void TriggerStorm(uint32_t mapId, uint32_t duration);",
        ],
        "settings":
            '    m_maxWeatherType = Aurora::Settings::Get<uint32_t>("{cfg}.MaxWeatherType", 11, "{name}");\n'
            '    m_defaultWeatherType = Aurora::Settings::Bounded<uint32_t>("{cfg}.DefaultWeatherType", 5, 0, m_maxWeatherType, "{name}");\n'
            '    m_maxStormDurationSecs = Aurora::Settings::Get<uint32_t>("{cfg}.MaxStormDurationSecs", 3600, "{name}");\n'
            '    m_stormDurationSecs = Aurora::Settings::Bounded<uint32_t>("{cfg}.StormDurationSecs", 300, 1, m_maxStormDurationSecs, "{name}");\n',
        "init_log":
            '    LOG_INFO("@NAME@", "WeatherSystem module initialized (storm {} s, default type {}, max type {})",\n'
            '        m_stormDurationSecs, m_defaultWeatherType, m_maxWeatherType);',
        "extra_impl": """
bool WeatherSystemManager::IsValidWeatherType(uint32_t weatherType) const
{
    switch (weatherType)
    {
        // The ids the 3.3.5 client actually knows; the range alone would also
        // accept the gaps between them, which make the client show nothing.
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 11:
            return weatherType <= m_maxWeatherType;
        default:
            return false;
    }
}

void WeatherSystemManager::SetWeather(uint32_t mapId, uint32_t weatherType)
{
    // TODO: push the change to the live map once Map::SetWeather is wired up.
    LOG_INFO("WeatherSystem", "Set weather type {} on map {}", weatherType, mapId);
}

void WeatherSystemManager::TriggerStorm(uint32_t mapId, uint32_t duration)
{
    LOG_INFO("WeatherSystem", "Triggered storm on map {} for {} seconds", mapId, duration);
}
""",
        "commands": [
            CMD(sub="set", handler="HandleSetWeatherCommand",
                args=", uint32 mapId, Optional<uint32> weatherTypeArg",
                pre='        uint32 const weatherType = weatherTypeArg.value_or(WeatherSystemManager::Instance().DefaultWeatherType());\n'
                    '        if (!WeatherSystemManager::Instance().IsValidWeatherType(weatherType))\n'
                    '        {\n'
                    '            handler->PSendSysMessage("Unknown weather type {} (0-8 or 11, up to {}).",\n'
                    '                weatherType, WeatherSystemManager::Instance().MaxWeatherType());\n'
                    '            return true;\n'
                    '        }\n\n',
                msg="Weather type {} set on map {}.",
                fmt="weatherType, mapId"),
            CMD(sub="storm", handler="HandleTriggerStormCommand",
                args=", uint32 mapId, Optional<uint32> durationArg",
                pre='        uint32 duration = durationArg.value_or(WeatherSystemManager::Instance().StormDurationSecs());\n'
                    '        uint32 const maxDuration = WeatherSystemManager::Instance().MaxStormDurationSecs();\n'
                    '        if (!duration || duration > maxDuration)\n'
                    '        {\n'
                    '            handler->PSendSysMessage("Storm duration must be between 1 and {} seconds.", maxDuration);\n'
                    '            return true;\n'
                    '        }\n\n',
                msg="Storm triggered on map {} for {} seconds.",
                fmt="mapId, duration"),
            CMD(sub="info", handler="HandleWeatherInfoCommand",
                msg="Default storm duration: {} s (maximum {}), default weather type {}.",
                fmt=f'{m("WeatherSystem")}.StormDurationSecs(), {m("WeatherSystem")}.MaxStormDurationSecs(), '
                    f'{m("WeatherSystem")}.DefaultWeatherType()'),
        ],
    },
]


def build_handler(cls, name, prefix, cmd: CMD):
    msg = cmd.get("msg", "")
    fmt = cmd.get("fmt", "")

    if "{}" in msg:
        body = 'handler->PSendSysMessage("' + msg + '"' + \
            (", " + fmt if fmt else "") + ");"
    else:
        body = 'handler->SendSysMessage("' + msg + '");'

    extra_guard = cmd.get("extra_guard", "")
    if extra_guard:
        extra_guard = (extra_guard.replace("@CLS@", cls)
                       .replace("{cls}", cls)
                       .replace("{mgr}", f"{cls}Manager::Instance()"))

    return HANDLER.format(
        prefix=prefix, sub=cmd["sub"], handler=cmd["handler"],
        argdecl=cmd.get("args", ", std::string_view /*args*/"),
        body=body, guard=GUARD.format(cls=cls, name=name),
        extra_guard=extra_guard,
        pre=cmd.get("pre", ""),
    )


def main():
    for spec in SPECS:
        name, cls, prefix, cfg = spec["name"], spec["cls"], spec["prefix"], spec["cfg"]

        table = "\n".join(
            '            {{ "{}", {}, rbac::RBAC_PERM_COMMAND_GM, Console::No }},'.format(
                cmd["sub"], cmd["handler"])
            for cmd in spec["commands"]
        )
        handlers = "\n".join(build_handler(cls, name, prefix, cmd) for cmd in spec["commands"])

        extra_impl = spec.get("extra_impl", "")
        text = TEMPLATE.format(
            name=name, cls=cls, prefix=prefix, human=spec["human"], blurb=spec["blurb"],
            conf=spec["conf"], cfg=cfg, table=table, handlers=handlers,
            loader="Add%sScripts" % name,
            load_settings=spec["settings"].format(cfg=cfg, name=name),
            init_log=spec["init_log"].replace("@NAME@", name),
            extra_impl=("\n" + extra_impl.strip() + "\n") if extra_impl else "",
        ).replace("@NAME@", name)  # the template's own log lines still carry the marker

        header = HEADER_FILE.format(
            name=name, cls=cls,
            public_extra="".join(line.rstrip() + NL for line in spec["public_extra"]),
            fields="".join(line.rstrip() + NL for line in spec["fields"]),
        )

        out = MODULES / name / "src" / f"{name}Module.cpp"
        out.write_text(text, encoding="utf-8")
        (MODULES / name / "src" / f"{name}Module.h").write_text(header, encoding="utf-8")
        print(f"  generated: {out.name} (+ header)")


if __name__ == "__main__":
    main()

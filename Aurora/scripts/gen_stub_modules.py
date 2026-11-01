"""
One-shot generator for the small Aurora stub modules.

Every module listed below has the same shape (a manager, a handful of
placeholder commands and a config-driven enable switch), so they are generated
from one template instead of hand-writing eight near-identical files. The
option names written here match the conf/*.conf.dist files produced by
scripts/gen_module_configs.py.

Run:  python scripts/gen_stub_modules.py
Then: python scripts/migrate_to_ac.py
"""
from pathlib import Path

MODULES = Path(__file__).resolve().parent.parent / "modules"

TEMPLATE = """/*
 * {name} module \u2014 implementation
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
// {cls}Manager \u2014 implementation
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
        LOG_INFO("{name}", "{name} disabled by {cfg}.Enabled");
        return true;
    }}

    LOG_INFO("{name}", "{name} module initialized");
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
    LOG_INFO("{name}", "Player {{}} logged in \u2014 {human} available", p->GetName());
}}

void {cls}Manager::OnPlayerLevelUp(Player* p)
{{
    if (!p || !m_enabled)
        return;
    LOG_INFO("{name}", "Player {{}} leveled up", p->GetName());
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
{handlers}
}};

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
        LOG_INFO("{name}", "{name} module loaded (scripts auto-register)");
    }}

    __declspec(dllexport) void __stdcall Finalize{name}Module()
    {{
        LOG_INFO("{name}", "{name} module finalized");
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

# A command is (subcommand, handler name, message, argument declarations,
# format arguments). The number of "{}" in the message must match the number of
# format arguments; a message without placeholders uses the plain overload.
CMD = tuple

# (module, manager class, chat prefix, human name, blurb, conf file,
#  config prefix, settings read in Initialize(), commands, extra impl)
SPECS = [
    (
        "AdminTools", "AdminTools", "admin", "admin tools",
        "GM helpers: admin panel, server info and an umbrella command.",
        "aurora_admintools.conf", "Aurora.AdminTools",
        '    m_requireSession = Aurora::Settings::Enabled("{cfg}.RequireSession", "{name}", true);\n',
        [
            ("adminpanel", "HandleAdminPanelCommand",
             "Admin panel is {}.", "", 'AdminToolsManager::Instance().RequireSession() ? "restricted to in-game GMs" : "also available from the console"'),
            ("serverinfo", "HandleServerInfoCommand", "Server info command received.", "", ""),
            ("help", "HandleAdminCommand", "Commands: .admin adminpanel, .admin serverinfo, .admin help.", "", ""),
        ],
        "",
    ),
    (
        "ArenaSystem", "ArenaSystem", "arena", "arena system",
        "Arena team helpers: queueing, ranks and per-team statistics.",
        "aurora_arena.conf", "Aurora.Arena",
        '    m_ratedMinLevel = Aurora::Settings::Get<uint32_t>("{cfg}.RatedMinimumLevel", 80, "{name}");\n',
        [
            ("queue", "HandleArenaQueueCommand", "Arena queue command received.", "", ""),
            ("rank", "HandleArenaRankCommand", "Arena rank command received.", "", ""),
            ("stats", "HandleArenaStatsCommand",
             "Rated arena requires level {}.", "", "ArenaSystemManager::Instance().RatedMinLevel()"),
        ],
        "",
    ),
    (
        "Economy", "Economy", "economy", "economy system",
        "Custom economy hooks: auctions, currencies and exchanges.",
        "aurora_economy.conf", "Aurora.Economy",
        '    m_auctionDepositPercent = Aurora::Settings::Get<uint32_t>("{cfg}.AuctionDepositPercent", 5, "{name}");\n',
        [
            ("auction", "HandleAuctionCommand",
             "Auction of bid {} needs a {} percent deposit.", "uint32 bid", "bid, EconomyManager::Instance().AuctionDepositPercent()"),
            ("currency", "HandleCurrencyCommand", "Currency command received.", "", ""),
            ("exchange", "HandleExchangeCommand", "Exchange command received.", "", ""),
        ],
        "",
    ),
    (
        "GuildEnhancements", "GuildEnhancements", "guildx", "guild enhancements",
        "Guild improvements: bank, ranks and extended info.",
        "aurora_guild.conf", "Aurora.GuildEnhancements",
        '    m_bankTabCost = Aurora::Settings::Get<uint32_t>("{cfg}.BankTabCost", 100000, "{name}");\n',
        [
            ("bank", "HandleGuildBankCommand",
             "Guild bank command received (a tab costs {}).", "",
             "GuildEnhancementsManager::Instance().BankTabCost()"),
            ("rank", "HandleGuildRankCommand", "Guild rank command received.", "", ""),
            ("info", "HandleGuildInfoCommand", "Guild info command received.", "", ""),
        ],
        "",
    ),
    (
        "LootSystem", "LootSystem", "loot", "loot system",
        "Loot tuning: drop rates, loot inspection and reset.",
        "aurora_loot.conf", "Aurora.Loot",
        '    m_dropRate = Aurora::Settings::Get<float>("{cfg}.DropRate", 1.0f, "{name}");\n',
        [
            ("droprate", "HandleSetDropRateCommand", "Drop rate set to {}.", "float rate", "rate"),
            ("info", "HandleLootInfoCommand", "Configured drop rate: {}.", "",
             "LootSystemManager::Instance().DropRate()"),
            ("reset", "HandleResetLootCommand", "Loot tables reset.", "", ""),
        ],
        "",
    ),
    (
        "PetSystem", "PetSystem", "pet", "pet system",
        "Custom pet handling: spawning, removal and stat overrides.",
        "aurora_pets.conf", "Aurora.Pets",
        '    m_defaultPetLevel = Aurora::Settings::Get<uint32_t>("{cfg}.DefaultLevel", 1, "{name}");\n',
        [
            ("spawn", "HandleSpawnPetCommand", "Pet entry {} spawned at level {}.", "uint32 entry",
             "entry, PetSystemManager::Instance().DefaultPetLevel()"),
            ("remove", "HandleRemovePetCommand", "Pet removed.", "", ""),
            ("level", "HandleSetPetLevelCommand", "Pet level set to {}.", "uint32 level", "level"),
            ("stats", "HandleSetPetStatsCommand", "Pet stats set to {}/{}.",
             "uint32 health, uint32 mana", "health, mana"),
        ],
        """
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
    ),
    (
        "QuestSystem", "QuestSystem", "questx", "quest system",
        "Custom quest helpers: creation, inspection and completion.",
        "aurora_quest.conf", "Aurora.Quests", "",
        [
            ("create", "HandleCreateQuestCommand", "Create quest command received.", "", ""),
            ("info", "HandleQuestInfoCommand", "Quest info command received.", "", ""),
            ("complete", "HandleCompleteQuestCommand", "Complete quest command received.", "", ""),
        ],
        "",
    ),
    (
        "WeatherSystem", "WeatherSystem", "weather", "weather system",
        "Map weather control: set a weather type or trigger a timed storm.",
        "aurora_weather.conf", "Aurora.Weather",
        '    m_stormDurationSecs = Aurora::Settings::Get<uint32_t>("{cfg}.StormDurationSecs", 300, "{name}");\n',
        [
            ("set", "HandleSetWeatherCommand", "Weather type {} set on map {}.",
             "uint32 mapId, uint32 weatherType", "weatherType, mapId"),
            ("storm", "HandleTriggerStormCommand", "Storm triggered on map {} for {} seconds.",
             "uint32 mapId, uint32 duration", "mapId, duration"),
            ("info", "HandleWeatherInfoCommand", "Default storm duration: {} s.", "",
             "WeatherSystemManager::Instance().StormDurationSecs()"),
        ],
        """
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
    ),
]

HANDLER = """    // ---------- .{prefix} {sub} ----------
    static bool {handler}(ChatHandler* handler{argdecl})
    {{
{guard}        {body}
        return true;
    }}
"""

GUARD = """        if (!{cls}Manager::Instance().IsEnabled())
        {{
            handler->SendSysMessage("{name} is disabled by configuration.");
            return true;
        }}

"""


def build_handler(cls, name, prefix, cmd: CMD):
    sub, handler, msg, argdecl, fmt_args = cmd

    if "{}" in msg:
        body = 'handler->PSendSysMessage("' + msg + '"' + \
            (", " + fmt_args if fmt_args else "") + ");"
    else:
        body = 'handler->SendSysMessage("' + msg + '");'

    return HANDLER.format(
        prefix=prefix, sub=sub, handler=handler,
        argdecl=", " + argdecl if argdecl else ", std::string_view /*args*/",
        body=body, guard=GUARD.format(cls=cls, name=name),
    )


def main():
    for name, cls, prefix, human, blurb, conf, cfg, settings, commands, extra_impl in SPECS:
        table = "\n".join(
            '            {{ "{}", {}, rbac::RBAC_PERM_COMMAND_GM, Console::No }},'.format(sub, handler)
            for sub, handler, _, _, _ in commands
        )
        handlers = "\n".join(build_handler(cls, name, prefix, cmd) for cmd in commands)

        text = TEMPLATE.format(
            name=name, cls=cls, prefix=prefix, human=human, blurb=blurb,
            conf=conf, cfg=cfg, table=table, handlers=handlers,
            loader="Add%sScripts" % name,
            load_settings=settings.format(cfg=cfg, name=name),
            extra_impl=("\n" + extra_impl.strip() + "\n") if extra_impl else "",
        )

        out = MODULES / name / "src" / f"{name}Module.cpp"
        out.write_text(text, encoding="utf-8")
        print(f"  generated: {out.name}")


if __name__ == "__main__":
    main()

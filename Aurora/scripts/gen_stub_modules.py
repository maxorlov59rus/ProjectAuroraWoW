"""
One-shot generator for the small Aurora stub modules.

Every module listed below has the same shape (a manager plus a handful of
placeholder commands), so they are generated from one template instead of
hand-writing seven near-identical files. Run:

    python build/_gen/gen_stub_modules.py

Afterwards the modules go through scripts/migrate_to_ac.py as usual.
"""
from pathlib import Path

MODULES = Path(r"C:/Users/Orel/.cline/worktrees/b823d/Cline/Aurora/modules")

TEMPLATE = """/*
 * {name} module \u2014 implementation
 *
 * {blurb}
 *
 * This module registers one AzerothCore script:
 *   - {cls}CommandScript : CommandScript (chat commands)
 */

#include "{name}Module.h"

#include <AuroraPCH.h>
#include <AuroraLogging.h>

#include <ScriptMgr.h>
#include <CommandScript.h>
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
    LOG_INFO("{name}", "{name} module initialized");
    return true;
}}

void {cls}Manager::OnPlayerLogin(Player* p)
{{
    if (!p)
        return;
    LOG_INFO("{name}", "Player {{}} logged in \u2014 {human} available", p->GetName());
}}

void {cls}Manager::OnPlayerLevelUp(Player* p)
{{
    if (!p)
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
// Module entry points
// ============================================================================

extern "C"
{{
    __declspec(dllexport) void __stdcall Initialize{name}Module()
    {{
        {cls}Manager::Instance().Initialize();
        LOG_INFO("{name}", "{name} module initialized (scripts auto-register)");
    }}

    __declspec(dllexport) void __stdcall Finalize{name}Module()
    {{
        LOG_INFO("{name}", "{name} module finalized");
    }}
}}

namespace
{{
    {cls}CommandScript s{cls}CommandScript;
}}
"""

# name -> (manager class, chat prefix, human name, blurb, commands)
# A command is (subcommand, handler name, message, extra args or "").
SPECS = [
    (
        "ArenaSystem", "ArenaSystem", "arena", "arena system",
        "Arena team helpers: queueing, ranks and per-team statistics.",
        [
            ("queue", "HandleArenaQueueCommand", "Arena queue command received.", ""),
            ("rank", "HandleArenaRankCommand", "Arena rank command received.", ""),
            ("stats", "HandleArenaStatsCommand", "Arena stats command received.", ""),
        ],
        "",
    ),
    (
        "Economy", "Economy", "economy", "economy system",
        "Custom economy hooks: auctions, currencies and exchanges.",
        [
            ("auction", "HandleAuctionCommand", "Auction command received.", ""),
            ("currency", "HandleCurrencyCommand", "Currency command received.", ""),
            ("exchange", "HandleExchangeCommand", "Exchange command received.", ""),
        ],
        "",
    ),
    (
        "GuildEnhancements", "GuildEnhancements", "guildx", "guild enhancements",
        "Guild improvements: bank, ranks and extended info.",
        [
            ("bank", "HandleGuildBankCommand", "Guild bank command received.", ""),
            ("rank", "HandleGuildRankCommand", "Guild rank command received.", ""),
            ("info", "HandleGuildInfoCommand", "Guild info command received.", ""),
        ],
        "",
    ),
    (
        "LootSystem", "LootSystem", "loot", "loot system",
        "Loot tuning: drop rates, loot inspection and reset.",
        [
            ("droprate", "HandleSetDropRateCommand", "Drop rate set to {{}}.", "float rate"),
            ("info", "HandleLootInfoCommand", "Loot info command received.", ""),
            ("reset", "HandleResetLootCommand", "Loot tables reset.", ""),
        ],
        "",
    ),
    (
        "PetSystem", "PetSystem", "pet", "pet system",
        "Custom pet handling: spawning, removal and stat overrides.",
        [
            ("spawn", "HandleSpawnPetCommand", "Pet entry {{}} spawned.", "uint32 entry"),
            ("remove", "HandleRemovePetCommand", "Pet removed.", ""),
            ("level", "HandleSetPetLevelCommand", "Pet level set to {{}}.", "uint32 level"),
            ("stats", "HandleSetPetStatsCommand", "Pet stats set to {{}}/{{}}.", "uint32 health, uint32 mana"),
        ],
        """
void PetSystemManager::SpawnPet(Player* p, uint32_t entry)
{
    if (!p)
        return;
    // TODO: create the pet creature through the map spawn helper.
    LOG_INFO("PetSystem", "Spawned pet entry {} for player {}", entry, p->GetName());
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
        [
            ("create", "HandleCreateQuestCommand", "Create quest command received.", ""),
            ("info", "HandleQuestInfoCommand", "Quest info command received.", ""),
            ("complete", "HandleCompleteQuestCommand", "Complete quest command received.", ""),
        ],
        "",
    ),
    (
        "WeatherSystem", "WeatherSystem", "weather", "weather system",
        "Map weather control: set a weather type or trigger a timed storm.",
        [
            ("set", "HandleSetWeatherCommand", "Weather type {} set on map {}.", "uint32 mapId, uint32 weatherType"),
            ("storm", "HandleTriggerStormCommand", "Storm triggered on map {} for {} seconds.", "uint32 mapId, uint32 duration"),
            ("info", "HandleWeatherInfoCommand", "Weather info command received.", ""),
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
        {body}
        return true;
    }}
"""


def build_handler(prefix, cmd):
    sub, handler, msg, extra = cmd
    argdecl = (", " + extra) if extra else ", std::string_view /*args*/"

    # Fill the {} placeholders of the message with the handler arguments.
    fmt_args = ""
    if extra:
        names = [a.strip().split()[-1] for a in extra.split(",")]
        fmt_args = ", " + ", ".join(names)

    body = 'handler->PSendSysMessage("{}"{});'.format(msg.replace("{{", "{").replace("}}", "}"), fmt_args) \
        if "{" in msg else 'handler->SendSysMessage("{}");'.format(msg)

    return HANDLER.format(prefix=prefix, sub=sub, handler=handler, argdecl=argdecl, body=body)


def main():
    for name, cls, prefix, human, blurb, commands, extra_impl in SPECS:
        table = "\n".join(
            '            {{ "{}", {}, rbac::RBAC_PERM_COMMAND_GM, Console::No }},'.format(sub, handler)
            for sub, handler, _, _ in commands
        )
        handlers = "\n".join(build_handler(prefix, cmd) for cmd in commands)

        text = TEMPLATE.format(
            name=name, cls=cls, prefix=prefix, human=human, blurb=blurb,
            table=table, handlers=handlers,
            extra_impl=("\n" + extra_impl.strip() + "\n") if extra_impl else "",
        )

        out = MODULES / name / "src" / f"{name}Module.cpp"
        out.write_text(text, encoding="utf-8")
        print(f"  generated: {out}")


if __name__ == "__main__":
    main()

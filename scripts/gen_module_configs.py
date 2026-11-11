"""
Generates the AzerothCore config files shipped with each Aurora module.

AzerothCore picks up module options from <module>/conf/*.conf.dist at configure
time: the file is copied next to the binaries and its name (without .dist) is
baked into the worldserver image, so the core loads it from
configs/modules/<name>.conf on startup. That mechanism only registers the file,
the options themselves have to match what the module reads through
sConfigMgr->GetOption(), so both sides are generated from one spec here.

Rule for this spec: every option listed here is read by the module AND changes
what the module does. Adding an entry without the matching code change is a bug
- the core only warns about unknown keys, so such an option would silently do
nothing.

Run:  python scripts/gen_module_configs.py
Then: python scripts/migrate_to_ac.py   (copies sources + creates working .conf)
"""
from pathlib import Path

MODULES = Path(__file__).resolve().parent.parent / "modules"

HEADER = """# ============================================================================
# {name} module configuration
#
# Loaded by worldserver from configs/modules/{file} after worldserver.conf.
# Options are read by the module in WorldScript::OnStartup, so a change needs a
# restart (or `.reload config` followed by a re-login where the module says so).
# ============================================================================

"""

# (module, file name, [(key, default, comment)])
SPECS = [
    ("AdminTools", "aurora_admintools.conf", [
        ("Aurora.AdminTools.Enabled", "1",
         "Enable the Aurora admin commands (.admin adminpanel, .admin serverinfo,\n"
         ".admin help). Disabled commands answer that the module is off."),
        ("Aurora.AdminTools.RequireSession", "1",
         "Refuse the commands when they come from the server console instead of an\n"
         "in-game GM session."),
    ]),
    ("ArenaSystem", "aurora_arena.conf", [
        ("Aurora.Arena.Enabled", "1",
         "Enable the Aurora arena helpers (.arena queue, .arena rank, .arena stats)."),
        ("Aurora.Arena.RatedMinimumLevel", "80",
         "Level a character needs before .arena queue reports it as rated-arena\n"
         "eligible. Range 1-80."),
    ]),
    ("AuroraBots", "aurora_bots.conf", [
        ("Aurora.Bots.Enabled", "1",
         "Enable the bot registry; with 0 the registry refuses to create bots."),
        ("Aurora.Bots.MaxBots", "20",
         "Number of bots the registry keeps at most (0 = registry stays empty)."),
        ("Aurora.Bots.AllowedClasses", "\"WARRIOR,PALADIN,HUNTER,ROGUE,PRIEST,DEATHKNIGHT,SHAMAN,MAGE,WARLOCK\"",
         "Comma separated class names a bot may be created with. An unknown or\n"
         "unlisted class makes the creation fail."),
        ("Aurora.Bots.AllowedRaces", "\"HUMAN,DRAENEI,NIGHT_ELF,GNOME,DWARF,ORC,UNDEAD,TAUREN,TROLL,BLOOD_ELF\"",
         "Comma separated race names a bot may be created with."),
        ("Aurora.Bots.DefaultScript", "\"scripts/bots/patrol.lua\"",
         "Behaviour script given to a bot created without an explicit one.\n"
         "Empty means the bot is registered without a script."),
        ("Aurora.Bots.StartRunning", "1",
         "Mark newly created bots as running, i.e. their AI starts immediately.\n"
         "With 0 they are registered stopped and need `.botai <guid> start`."),
    ]),
    ("AuroraGameModes", "aurora_gamemodes.conf", [
        ("Aurora.GameModes.Enabled", "1",
         "Load the Aurora game-mode scenarios at startup."),
        ("Aurora.GameModes.Allowed", "\"standard,rest,test\"",
         "Scenario ids the module accepts. A value outside this list is reported\n"
         "at startup and the first allowed id is used instead."),
        ("Aurora.GameModes.Default", "\"standard\"",
         "Scenario selected at startup; it has to be one of Allowed above."),
    ]),
    ("AuroraSkills", "aurora_skills.conf", [
        ("Aurora.Skills.Enabled", "1",
         "Load the Aurora skill registry at startup."),
        ("Aurora.Skills.LogHooks", "0",
         "Write a line to the module log every time the experience or skill hooks\n"
         "are called. Useful while the skill rules themselves are still empty."),
    ]),
    ("AuroraSkins", "aurora_skins.conf", [
        ("Aurora.Skins.Enabled", "1",
         "Load the Aurora skin registry at startup."),
        ("Aurora.Skins.Default", "\"default\"",
         "Skin id returned for characters without an explicit skin. There is no\n"
         "skin table yet, so \"default\" is currently the only id in use."),
    ]),
    ("CampaignOfDarkness", "aurora_campaign_darkness.conf", [
        ("Aurora.CampaignOfDarkness.Enabled", "1",
         "Enable the Horde campaign (.cod commands and login hooks)."),
        ("Aurora.CampaignOfDarkness.SpawnListFile", "\"\"",
         "Spawn list loaded at startup; empty means the campaign starts empty and\n"
         "is filled through the console. Relative paths are resolved by worldserver."),
        ("Aurora.CampaignOfDarkness.RequireHorde", "1",
         "Only let Horde characters start the campaign."),
        ("Aurora.CampaignOfDarkness.MinLevelToUnlock", "10",
         "Character level from which the campaign is offered on login."),
        ("Aurora.CampaignOfDarkness.DefaultFaction", "2142",
         "Faction id given to campaign NPCs whose own faction is unspecified.\n"
         "2142 is the Horde campaign faction used by the shipped spawn data."),
        ("Aurora.CampaignOfDarkness.RewardGoldMultiplier", "1.0",
         "Multiplier applied to the gold reward reported when a quest is completed."),
    ]),
    ("CampaignOfLight", "aurora_campaign_light.conf", [
        ("Aurora.CampaignOfLight.Enabled", "1",
         "Enable the Alliance campaign (.cnp commands and login hooks)."),
        ("Aurora.CampaignOfLight.Faction", "\"alliance\"",
         "Faction value the campaign rows are selected by in the database tables."),
        ("Aurora.CampaignOfLight.IncludeNeutral", "1",
         "Also load rows whose faction is \"neutral\" alongside the ones above."),
        ("Aurora.CampaignOfLight.PreloadOnStartup", "0",
         "Read the campaign tables at startup instead of on the first command.\n"
         "Useful when the campaign must be available to players right away."),
        ("Aurora.CampaignOfLight.RequireAlliance", "1",
         "Only let Alliance characters start the campaign."),
        ("Aurora.CampaignOfLight.MinLevelToUnlock", "10",
         "Character level from which the campaign is offered on login."),
        ("Aurora.CampaignOfLight.BossRespawnMultiplier", "1.0",
         "Multiplier applied to the respawn time stored for a campaign boss."),
    ]),
    ("ClasslessSystem", "aurora_classless.conf", [
        ("Aurora.Classless.Enabled", "1",
         "Enable the classless registry and the .classless commands."),
        ("Aurora.Classless.PointsPerLevel", "2",
         "Skill points a classless character gets for each of its levels; they are\n"
         "granted when the character is registered. Range 0-10."),
        ("Aurora.Classless.MaxLevel", "80",
         "Highest level a classless character may be registered with."),
        ("Aurora.Classless.MaxSkillPoints", "0",
         "Ceiling for the skill points of one character, 0 = unlimited."),
        ("Aurora.Classless.DefaultScript", "\"\"",
         "Lua script recorded for a character created without an explicit one."),
    ]),
    ("Economy", "aurora_economy.conf", [
        ("Aurora.Economy.Enabled", "1",
         "Enable the Aurora economy helpers (.economy commands)."),
        ("Aurora.Economy.AuctionDepositPercent", "5",
         "Percent of a bid that the Aurora auction helper reports as deposit.\n"
         "Range 0-100."),
    ]),
    ("GuildEnhancements", "aurora_guild.conf", [
        ("Aurora.GuildEnhancements.Enabled", "1",
         "Enable the Aurora guild helpers (.guildx commands)."),
        ("Aurora.GuildEnhancements.BankTabCost", "100000",
         "Price in copper that the Aurora guild-bank helper charges for a new tab\n"
         "(100000 = 10 gold)."),
    ]),
    ("LootSystem", "aurora_loot.conf", [
        ("Aurora.Loot.Enabled", "1",
         "Enable the Aurora loot helpers (.loot commands)."),
        ("Aurora.Loot.DropRate", "1.0",
         "Drop-rate multiplier the module starts with; .loot droprate changes it at\n"
         "runtime. 1.0 = the rates as stored in the loot templates. This is an\n"
         "Aurora-side rate and does not touch the core Rate.Drop.Mobs.* settings."),
        ("Aurora.Loot.MaxDropRate", "10.0",
         "Upper bound for both the configured value and .loot droprate, so a typo\n"
         "cannot multiply drops by thousands."),
    ]),
    ("NpcCustomizer", "aurora_npccustomizer.conf", [
        ("Aurora.NpcCustomizer.Enabled", "1",
         "Enable the Aurora NPC customizer (.npc commands)."),
        ("Aurora.NpcCustomizer.MaxSpawnedNpcs", "100",
         "Number of customized NPCs the module keeps track of at once (0 = no NPC\n"
         "can be spawned). The registry is in memory and resets on restart."),
        ("Aurora.NpcCustomizer.DefaultName", "\"NPC\"",
         "Name shown for a spawned NPC created without an explicit one."),
        ("Aurora.NpcCustomizer.MaxLevel", "83",
         "Highest level that can be set on a customized NPC. 83 is the classic\n"
         "world-boss ceiling; raise it only for custom content."),
        ("Aurora.NpcCustomizer.MaxNameLength", "24",
         "Longest name accepted for a customized NPC."),
    ]),
    ("PetSystem", "aurora_pets.conf", [
        ("Aurora.Pets.Enabled", "1",
         "Enable the Aurora pet helpers (.pet commands)."),
        ("Aurora.Pets.DefaultLevel", "1",
         "Level given to a pet spawned without an explicit one."),
        ("Aurora.Pets.MaxLevel", "80",
         "Highest level that can be given to a pet by .pet level or .pet spawn."),
    ]),
    ("QuestSystem", "aurora_quest.conf", [
        ("Aurora.Quests.Enabled", "1",
         "Enable the Aurora quest helpers (.questx commands).\n"
         "The commands are placeholders for now, so this module has no further\n"
         "option until the quest creation itself is implemented."),
    ]),
    ("Teleportation", "aurora_teleportation.conf", [
        ("Aurora.Teleportation.Enabled", "1",
         "Enable the Aurora teleport destinations."),
        ("Aurora.Teleportation.LoadDefaults", "1",
         "Register the built-in capital destinations at startup. Disable it when the\n"
         "destinations come from a database table instead."),
        ("Aurora.Teleportation.Capitals", "\"Stormwind,Orgrimmar,Ironforge,Undercity,Thunder Bluff\"",
         "Which of the built-in destinations LoadDefaults registers, by name.\n"
         "Names are matched case-insensitively; an unknown name is reported at startup."),
    ]),
    ("WeatherSystem", "aurora_weather.conf", [
        ("Aurora.Weather.Enabled", "1",
         "Enable the Aurora weather overrides (.weather commands)."),
        ("Aurora.Weather.StormDurationSecs", "300",
         "Storm length used by .weather storm when it is called without one."),
        ("Aurora.Weather.MaxStormDurationSecs", "3600",
         "Longest storm duration accepted, from the config and from the command."),
        ("Aurora.Weather.DefaultWeatherType", "5",
         "Weather type used by .weather set without an explicit type.\n"
         "3.3.5 ids: 0 fine, 1 rain, 2 snow, 3 blackcloud, 4 rainstorm,\n"
         "5 thunderstorm, 6 sandstorm, 7 snowstorm, 8 blizzard, 11 heiter."),
        ("Aurora.Weather.MaxWeatherType", "11",
         "Highest weather id the module accepts."),
    ]),
    ("ZombieApocalypse", "aurora_zombie.conf", [
        ("Aurora.ZombieApocalypse.Enabled", "1",
         "Enable the zombie apocalypse event and its commands."),
        ("Aurora.ZombieApocalypse.DurationMinutes", "30",
         "Event length used by .zombie start without an explicit one."),
        ("Aurora.ZombieApocalypse.MaxDurationMinutes", "1440",
         "Upper bound for the event length, from the config and from the command."),
        ("Aurora.ZombieApocalypse.MaxZombies", "50",
         "Living zombies the event keeps at most, and the default for .zombie start."),
        ("Aurora.ZombieApocalypse.MaxZombieLevel", "80",
         "Highest level a randomly spawned zombie can get. Level is drawn in\n"
         "[1, MaxZombieLevel]."),
        ("Aurora.ZombieApocalypse.HealthPerLevel", "10",
         "Health a zombie gets for each of its levels."),
    ]),
]


def main():
    for name, file_name, options in SPECS:
        conf_dir = MODULES / name / "conf"
        conf_dir.mkdir(parents=True, exist_ok=True)

        lines = [HEADER.format(name=name, file=file_name).rstrip("\n"), ""]
        for key, default, comment in options:
            # AC-style doc block: option name, then an indented description.
            lines.append("#    %s" % key)
            for cline in comment.split("\n"):
                lines.append("#        %s" % cline.strip())
            lines.append("#        Default:     %s" % default)
            lines.append("")
            lines.append("%s = %s" % (key, default))
            lines.append("")

        (conf_dir / f"{file_name}.dist").write_text("\n".join(lines), encoding="utf-8")
        print(f"  {name}/conf/{file_name}.dist ({len(options)} options)")


if __name__ == "__main__":
    main()

"""
Generates the AzerothCore config files shipped with each Aurora module.

AzerothCore picks up module options from <module>/conf/*.conf.dist at configure
time: the file is copied next to the binaries and its name (without .dist) is
baked into the worldserver image, so the core loads it from
configs/modules/<name>.conf on startup. That mechanism only registers the file,
the options themselves have to match what the module reads through
sConfigMgr->GetOption(), so both sides are generated from one spec here.

Run:  python scripts/gen_module_configs.py
Then: python scripts/migrate_to_ac.py   (copies sources + creates working .conf)
"""
from pathlib import Path

MODULES = Path(__file__).resolve().parent.parent / "modules"

HEADER = """# ============================================================================
# {name} module configuration
#
# Loaded by worldserver from configs/modules/{file} after worldserver.conf.
# Options are read by the module through sConfigMgr->GetOption(), so they can
# be changed at runtime with `.reload config` unless noted otherwise.
# ============================================================================

"""

# (module, file name, [(key, default, comment)])
SPECS = [
    ("AdminTools", "aurora_admintools.conf", [
        ("Aurora.AdminTools.Enabled", "1",
         "Enable the Aurora admin commands (.admin adminpanel, .admin serverinfo)."),
        ("Aurora.AdminTools.RequireSession", "1",
         "Reject the commands when they are issued from the console rather than by an in-game GM."),
    ]),
    ("ArenaSystem", "aurora_arena.conf", [
        ("Aurora.Arena.Enabled", "1",
         "Enable the Aurora arena helpers (.arena queue, .arena rank, .arena stats)."),
        ("Aurora.Arena.RatedMinimumLevel", "80",
         "Minimum level for a character to be reported as rated-arena eligible."),
    ]),
    ("AuroraBots", "aurora_bots.conf", [
        ("Aurora.Bots.Enabled", "1",
         "Enable the bot registry and the .bot commands."),
        ("Aurora.Bots.MaxBots", "20",
         "Maximum number of bots the registry accepts (0 = no bots)."),
    ]),
    ("AuroraGameModes", "aurora_gamemodes.conf", [
        ("Aurora.GameModes.Enabled", "1",
         "Load the Aurora game-mode scenarios at startup."),
        ("Aurora.GameModes.Default", "\"standard\"",
         "Scenario started automatically at startup. Known ids: standard, rest, test."),
    ]),
    ("AuroraSkills", "aurora_skills.conf", [
        ("Aurora.Skills.Enabled", "1",
         "Load the Aurora skill registry at startup."),
    ]),
    ("AuroraSkins", "aurora_skins.conf", [
        ("Aurora.Skins.Enabled", "1",
         "Load the Aurora skin registry at startup."),
        ("Aurora.Skins.Default", "\"default\"",
         "Skin id reported for characters without an explicit skin."),
    ]),
    ("CampaignOfDarkness", "aurora_campaign_darkness.conf", [
        ("Aurora.CampaignOfDarkness.Enabled", "1",
         "Enable the Horde campaign (.cod commands and login hooks)."),
        ("Aurora.CampaignOfDarkness.SpawnListFile", "\"\"",
         "Optional spawn-list file loaded at startup; empty means the campaign\n"
         "starts empty and is filled through the console or the database."),
    ]),
    ("CampaignOfLight", "aurora_campaign_light.conf", [
        ("Aurora.CampaignOfLight.Enabled", "1",
         "Enable the Alliance campaign (.cnp commands and login hooks)."),
    ]),
    ("ClasslessSystem", "aurora_classless.conf", [
        ("Aurora.Classless.Enabled", "1",
         "Enable the classless registry and the .classless commands."),
        ("Aurora.Classless.PointsPerLevel", "2",
         "Skill points granted to a classless character for every level gained."),
    ]),
    ("Economy", "aurora_economy.conf", [
        ("Aurora.Economy.Enabled", "1",
         "Enable the Aurora economy helpers (.economy commands)."),
        ("Aurora.Economy.AuctionDepositPercent", "5",
         "Percent of the bid kept as an auction deposit by the Aurora auction helper."),
    ]),
    ("GuildEnhancements", "aurora_guild.conf", [
        ("Aurora.GuildEnhancements.Enabled", "1",
         "Enable the Aurora guild helpers (.guildx commands)."),
        ("Aurora.GuildEnhancements.BankTabCost", "100000",
         "Copper price charged by the Aurora guild-bank helper for a new tab."),
    ]),
    ("LootSystem", "aurora_loot.conf", [
        ("Aurora.Loot.Enabled", "1",
         "Enable the Aurora loot helpers (.loot commands)."),
        ("Aurora.Loot.DropRate", "1.0",
         "Default drop-rate multiplier applied by .loot droprate at startup.\n"
         "1.0 = vanilla rates. This is an Aurora-side rate, it does not change\n"
         "the core Rate.Drop.Mobs.* settings."),
    ]),
    ("NpcCustomizer", "aurora_npccustomizer.conf", [
        ("Aurora.NpcCustomizer.Enabled", "1",
         "Enable the Aurora NPC customizer (.npc commands)."),
    ]),
    ("PetSystem", "aurora_pets.conf", [
        ("Aurora.Pets.Enabled", "1",
         "Enable the Aurora pet helpers (.pet commands)."),
        ("Aurora.Pets.DefaultLevel", "1",
         "Level given to a pet spawned without an explicit level."),
    ]),
    ("QuestSystem", "aurora_quest.conf", [
        ("Aurora.Quests.Enabled", "1",
         "Enable the Aurora quest helpers (.questx commands)."),
    ]),
    ("Teleportation", "aurora_teleportation.conf", [
        ("Aurora.Teleportation.Enabled", "1",
         "Enable the Aurora teleportation destinations and commands."),
        ("Aurora.Teleportation.LoadDefaults", "1",
         "Register the built-in capital-city destinations at startup.\n"
         "Disable it when the destinations come from a database table instead."),
    ]),
    ("WeatherSystem", "aurora_weather.conf", [
        ("Aurora.Weather.Enabled", "1",
         "Enable the Aurora weather overrides (.weather commands)."),
        ("Aurora.Weather.StormDurationSecs", "300",
         "Default storm duration used when .weather storm is called without one."),
    ]),
    ("ZombieApocalypse", "aurora_zombie.conf", [
        ("Aurora.ZombieApocalypse.Enabled", "1",
         "Enable the zombie apocalypse event and its commands."),
        ("Aurora.ZombieApocalypse.DurationMinutes", "30",
         "Default event length for .zombie start."),
        ("Aurora.ZombieApocalypse.MaxZombies", "50",
         "Default living-zombie cap for the event."),
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
        print(f"  {name}/conf/{file_name}.dist")


if __name__ == "__main__":
    main()

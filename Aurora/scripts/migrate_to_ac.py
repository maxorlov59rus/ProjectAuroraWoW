"""
Migration: Aurora -> AzerothCore module mechanism.

Idempotent. Run: python build/_migrate/migrate.py
"""
import shutil
import sys
from pathlib import Path

AURORA = Path(r"C:/Users/Orel/.cline/worktrees/b823d/Cline/Aurora")
AC = Path(r"E:/azerothcore-wotlk-master")

# Modules whose sources compile into worldserver. A module moves here once it
# builds clean; anything else stays "disabled" so it cannot break the build.
ENABLED = [
    "AdminTools",
    "ArenaSystem",
    "AuroraBots",
    "AuroraGameModes",
    "AuroraSkills",
    "AuroraSkins",
    "CampaignOfDarkness",
    "CampaignOfLight",
    "ClasslessSystem",
    "Economy",
    "GuildEnhancements",
    "LootSystem",
    "NpcCustomizer",
    "PetSystem",
    "QuestSystem",
    "Teleportation",
    "WeatherSystem",
    "ZombieApocalypse",
]

SRC_SUFFIXES = (".cpp", ".h")


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="surrogateescape")


def write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", errors="surrogateescape")


def patch(path: Path, old: str, new: str) -> None:
    text = read(path)
    if new in text:
        print(f"  skip (already patched): {path.name}")
        return
    if old not in text:
        print(f"  ERROR: anchor not found in {path}")
        sys.exit(1)
    write(path, text.replace(old, new, 1))
    print(f"  patched: {path}")


def step_hook_into_ac_build():
    print("[1/6] Hooking AuroraCore into the AzerothCore build")

    patch(
        AC / "src" / "server" / "CMakeLists.txt",
        "if (BUILD_APPLICATION_WORLDSERVER)\n  add_subdirectory(game)",
        "if (BUILD_APPLICATION_WORLDSERVER)\n  add_subdirectory(AuroraCore)\n  add_subdirectory(game)",
    )

    # game-interface is what AzerothCore exposes to scripts and modules, so
    # AuroraCore becomes visible to every module through it (include dirs come
    # from the PUBLIC include directory of the AuroraCore target).
    patch(
        AC / "src" / "server" / "game" / "CMakeLists.txt",
        "target_link_libraries(game-interface\n  INTERFACE\n    shared)",
        "target_link_libraries(game-interface\n  INTERFACE\n    shared\n    AuroraCore)",
    )


def module_sources(mod: Path):
    src = mod / "src"
    base = src if src.is_dir() else mod
    return [f for f in sorted(base.iterdir()) if f.suffix in SRC_SUFFIXES]


def adapt_to_ac(text: str) -> str:
    """Adapts Aurora sources to the AzerothCore API they compile against.

    Aurora sources were written against an older/different core API. These
    rewrites are mechanical, so they live in the migration step instead of
    being duplicated across every module.
    """
    # LOG_INFO/LOG_ERROR are AzerothCore macros from Log.h. AuroraLogging.h
    # only declares the Aurora::Logging wrapper, so a module that calls
    # LOG_INFO must include Log.h itself.
    if "LOG_INFO(" in text or "LOG_ERROR(" in text:
        if '#include "Log.h"' not in text:
            text = '#include "Log.h"\n' + text

    # AzerothCore renamed the level accessor.
    text = text.replace("->getLevel()", "->GetLevel()")

    return text


def loader_snippet(dir_name: str) -> str:
    fn = "Add%sScripts" % dir_name.replace("-", "_")
    return """

// ============================================================================
// AzerothCore module loader entry point.
//
// The generated ModulesLoader.cpp calls this function at worldserver startup.
// The scripts of this module register themselves through their static
// instances, so the body only has to exist: referencing the function is what
// makes the linker pull this translation unit into the final image, and that
// is what triggers those static registrations.
// ============================================================================

void %s()
{
}
""" % fn


def step_copy_modules():
    print("[2/6] Copying Aurora modules into AC modules/")

    for mod in sorted(p for p in (AURORA / "modules").iterdir() if p.is_dir()):
        dst = AC / "modules" / mod.name / "src"
        dst.mkdir(parents=True, exist_ok=True)

        copied = []
        for f in module_sources(mod):
            text = read(f)
            if f.suffix == ".cpp":
                fn = "Add%sScripts" % mod.name.replace("-", "_")
                if fn not in text:
                    text = text.rstrip() + loader_snippet(mod.name)
            write(dst / f.name, adapt_to_ac(text))
            copied.append(f.name)

        # Config templates go along with the sources: AC's modules/CMakeLists
        # globs <module>/conf/*.conf.dist at configure time to build
        # CONFIG_FILE_LIST, which is what makes worldserver load the file.
        conf_src = mod / "conf"
        if conf_src.is_dir():
            conf_dst = AC / "modules" / mod.name / "conf"
            conf_dst.mkdir(parents=True, exist_ok=True)
            for f in sorted(conf_src.glob("*.conf.dist")):
                write(conf_dst / f.name, read(f))

        # NOTE: no CMakeLists.txt on purpose. AC's module loader collects
        # modules/<name>/src/*.{cpp,h} itself (static linkage), and the root
        # CMakeLists would add_subdirectory() a module CMakeLists.txt and
        # create a second, duplicate target.
        print(f"  {mod.name}: {', '.join(copied) if copied else 'NO SOURCES'}")


def step_module_selection():
    print("[3/6] Writing AC conf/config.cmake (module selection)")

    mods = sorted(p.name for p in (AURORA / "modules").iterdir() if p.is_dir())
    disabled = [m for m in mods if m not in ENABLED]

    lines = [
        "#",
        "# Aurora flavor - custom configuration for the AzerothCore build system.",
        "#",
        "# Picked up by AzerothCore's root CMakeLists.txt right after",
        "# conf/dist/config.cmake; the supported place to override options",
        "# without touching upstream files.",
        "#",
        "# Aurora modules live in modules/ and are built with AzerothCore's own",
        "# module mechanism. Modules not ported yet are disabled explicitly so",
        "# they don't silently break the build.",
        "#",
        "",
        "set(AURORA_ENABLED_MODULES",
    ]
    lines += ["    %s" % m for m in ENABLED]
    lines += [")", "", "foreach(_aurora_mod ${AURORA_ENABLED_MODULES})",
              "    string(TOUPPER ${_aurora_mod} _aurora_mod_upper)",
              '    set(MODULE_${_aurora_mod_upper} "static" CACHE STRING',
              '        "Aurora: build ${_aurora_mod} statically into worldserver" FORCE)',
              "endforeach()", "",
              "set(AURORA_DISABLED_MODULES"]
    lines += ["    %s" % m for m in disabled]
    lines += [")", "", "foreach(_aurora_mod ${AURORA_DISABLED_MODULES})",
              "    string(TOUPPER ${_aurora_mod} _aurora_mod_upper)",
              '    set(MODULE_${_aurora_mod_upper} "disabled" CACHE STRING',
              '        "Aurora: ${_aurora_mod} not built yet (pending port)" FORCE)',
              "endforeach()", ""]

    write(AC / "conf" / "config.cmake", "\n".join(lines))
    print("  wrote AC/conf/config.cmake: enabled=%s disabled=%d"
          % (",".join(ENABLED), len(disabled)))


def step_write_configs():
    """Creates the working .conf files next to the binaries.

    AzerothCore only ships *.conf.dist: worldserver refuses to start without
    configs/worldserver.conf, and it looks for the module options in
    configs/modules/<name>.conf. CopyModuleConfig() copies the module files
    verbatim (keeping the .dist suffix), so the real files are created here.
    Existing files are never overwritten - they hold local settings.
    """
    print("[4/6] Creating working config files in the build tree")

    bins = sorted(p for p in (AC / "build" / "bin").iterdir() if p.is_dir())
    if not bins:
        print("  WARNING: no build/bin/<config> directory yet - run cmake --build first")
        return

    created = updated = skipped = 0
    for cfg_dir in bins:
        conf_dir = cfg_dir / "configs"
        if not conf_dir.is_dir():
            continue

        # (source, destination, overwrite)
        #
        # worldserver.conf/authserver.conf are kept untouched once created: the
        # working copies carry this machine's database credentials and paths, and
        # overwriting them with the .dist would undo that. The module configs are
        # pure generator output, so they are refreshed on every run - otherwise a
        # new option never reaches the server that has to read it.
        todo = []
        for app in ("worldserver", "authserver"):
            dist = conf_dir / f"{app}.conf.dist"
            if dist.is_file():
                todo.append((dist, conf_dir / f"{app}.conf", False))

        modules_conf = conf_dir / "modules"
        modules_conf.mkdir(exist_ok=True)
        for dist in sorted((AURORA / "modules").glob("*/conf/*.conf.dist")):
            todo.append((dist, modules_conf / dist.name[: -len(".dist")], True))

        for src, dst, overwrite in todo:
            existed = dst.exists()
            if existed and not overwrite:
                skipped += 1
                continue

            if existed and read(dst) == read(src):
                skipped += 1
                continue

            write(dst, read(src))
            if existed:
                updated += 1
            else:
                created += 1
            print(f"  {'updated' if existed else 'created'}: {dst.relative_to(cfg_dir)}")

    print(f"  configs: {created} created, {updated} refreshed, {skipped} already current")


def step_copy_runtime_dlls():
    """Puts the runtime DLLs worldserver loads next to its own exe.

    Linking the modules statically removes the script DLLs, but three load-time
    dependencies stay dynamic and are not on PATH: libmysql.dll (the client
    library MySQL's server package ships) and, for OpenSSL 3+/4, libcrypto plus
    the 'legacy' provider OpenSSLCrypto::SetupLibrariesForWindows loads
    explicitly - without it worldserver asserts before it can log anything.
    Files already in place are left untouched so a manual copy wins.
    """
    print("[5/6] Copying runtime DLLs into build/bin/<config>")

    candidates = [
        ("libmysql.dll", [
            AC / "env" / "server" / "bin" / "libmysql.dll",
            Path(r"C:/Program Files/MySQL/MySQL Server 8.0/lib/libmysql.dll"),
            Path(r"C:/Program Files/MySQL/MySQL Server 8.4/lib/libmysql.dll"),
        ]),
        # OpenSSL names the crypto/ssl libraries after their ABI (libcrypto-3,
        # -4, ...), so those two are matched by pattern inside a directory.
        ("libcrypto*x64.dll", [Path(r"C:/Program Files/OpenSSL-Win64/bin")]),
        ("libssl*x64.dll", [Path(r"C:/Program Files/OpenSSL-Win64/bin")]),
        ("legacy.dll", [Path(r"C:/Program Files/OpenSSL-Win64/bin")]),
    ]

    bins = sorted(p for p in (AC / "build" / "bin").iterdir() if p.is_dir()) \
        if (AC / "build" / "bin").is_dir() else []
    if not bins:
        print("  WARNING: no build/bin/<config> directory yet - run cmake --build first")
        return

    for cfg_dir in bins:
        for name, sources in candidates:
            found = next((s for s in sources if s.is_file()), None)
            if found is None:
                for directory in sources:
                    if directory.is_dir():
                        matches = sorted(directory.glob(name))
                        if matches:
                            found = matches[0]
                            break

            dst = cfg_dir / found.name if found else None
            if found is None:
                print(f"  WARNING: {name} not found - worldserver cannot start without it")
                continue
            if dst.exists():
                continue

            shutil.copy2(found, dst)
            print(f"  copied: {found.name} ({cfg_dir.name})")


def step_check_auroracore():
    print("[6/6] Syncing AuroraCore into AC src/server/AuroraCore")

    core = AC / "src" / "server" / "AuroraCore"
    core.mkdir(parents=True, exist_ok=True)

    # Sources are authored under Aurora/src/Aurora; the AC tree keeps a copy so
    # that AC's own CMakeLists (which resolves paths against CMAKE_SOURCE_DIR)
    # can build it. The CMakeLists.txt in AC is hand-maintained (PUBLIC include
    # dir) and is deliberately not overwritten.
    copied = 0
    for f in sorted((AURORA / "src" / "Aurora").iterdir()):
        if f.suffix not in (".cpp", ".h"):
            continue
        dst = core / f.name
        if not dst.exists() or read(dst) != read(f):
            write(dst, read(f))
            copied += 1
    print("  %s: %d file(s) refreshed" % (core, copied))

    files = sorted(f.name for f in core.iterdir())
    if "CMakeLists.txt" not in files:
        print("  ERROR: AuroraCore/CMakeLists.txt missing")
        sys.exit(1)


if __name__ == "__main__":
    if not AC.exists():
        print("AC tree not found: %s" % AC)
        sys.exit(1)
    step_hook_into_ac_build()
    step_copy_modules()
    step_module_selection()
    step_write_configs()
    step_copy_runtime_dlls()
    step_check_auroracore()
    print("done")

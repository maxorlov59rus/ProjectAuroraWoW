"""
Migration: Aurora -> AzerothCore module mechanism.

Idempotent. Run: python build/_migrate/migrate.py
"""
import sys
from pathlib import Path

AURORA = Path(r"C:/Users/Orel/.cline/worktrees/b823d/Cline/Aurora")
AC = Path(r"E:/azerothcore-wotlk-master")

ENABLED = ["CampaignOfLight"]

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
    print("[1/4] Hooking AuroraCore into the AzerothCore build")

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
    print("[2/4] Copying Aurora modules into AC modules/")

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
            write(dst / f.name, text)
            copied.append(f.name)

        # NOTE: no CMakeLists.txt on purpose. AC's module loader collects
        # modules/<name>/src/*.{cpp,h} itself (static linkage), and the root
        # CMakeLists would add_subdirectory() a module CMakeLists.txt and
        # create a second, duplicate target.
        print(f"  {mod.name}: {', '.join(copied) if copied else 'NO SOURCES'}")


def step_module_selection():
    print("[3/4] Writing AC conf/config.cmake (module selection)")

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


def step_check_auroracore():
    print("[4/4] Checking AuroraCore placement")
    core = AC / "src" / "server" / "AuroraCore"
    files = sorted(f.name for f in core.iterdir())
    print("  %s: %d files" % (core, len(files)))
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
    step_check_auroracore()
    print("done")

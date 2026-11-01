# ============================================================================
# Aurora  –  custom flavor of AzerothCore (World of Warcraft Wotlk)
#
# This project is NOT a rewrite of AzerothCore. It is a "flavor" (distribution)
# built ON TOP of AzerothCore:
#   https://github.com/azerothcore/azerothcore-wotlk
#
# Aurora adds:
#   - custom game modes / roleplay rules      -> src/Aurora
#   - custom skills / skill handling          -> modules/AuroraSkills
#   - custom character skins                  -> modules/AuroraSkins
#   - client UI / interface tweaks            -> docs/AURORA_CUSTOMIZATION.md
#
# The actual server (WorldServer, AuthServer) and all client communication
# logic come from AzerothCore. The Aurora launcher entry points are provided
# by AzerothCore itself.
#
# Build:
#   CMakePresets.json  (Visual Studio / CLion / VS Code)
#   scripts/build_win.bat
#   scripts/build_linux.sh
# ============================================================================

cmake_minimum_required(VERSION 3.16...3.22)

# --- AzerothCore source tree location -------------------------------
# Override at configure time, e.g.:
#   cmake -DAZEROTHCORE_SOURCE_DIR=E:/azerothcore-wotlk-master ..
if(NOT AZEROTHCORE_SOURCE_DIR)
    set(AZEROTHCORE_SOURCE_DIR "E:/azerothcore-wotlk-master"
        CACHE PATH "Absolute path to the AzerothCore source tree")
endif()

if(NOT EXISTS "${AZEROTHCORE_SOURCE_DIR}")
    message(FATAL_ERROR
        "AzerothCore source directory not found:\n  ${AZEROTHCORE_SOURCE_DIR}\n"
        "Point AZEROTHCORE_SOURCE_DIR to the correct path.")
endif()

# Directory used to install the Aurora modules (relative to the build tree).
set(AURORA_MODULE_DIR "modules" CACHE PATH "Build directory for Aurora modules")

message(STATUS "== Aurora flavor project ==")
message(STATUS "AzerothCore source: ${AZEROTHCORE_SOURCE_DIR}")

# --- Load AzerothCore's own build system ------------------------------
# NOTE: AzerothCore's root CMakeLists internally calls project(AzerothCore ...)
# so the resulting binaries are named worldserver / authserver.
add_subdirectory(${AZEROTHCORE_SOURCE_DIR})

# --- Aurora custom C++ layer (server side) ---------------------------
add_subdirectory(src/Aurora)

# --- Aurora modules (loadable by the AzerothCore module loader) -------
add_subdirectory(modules/AuroraGameModes)
add_subdirectory(modules/AuroraSkills)
add_subdirectory(modules/ClasslessSystem)
add_subdirectory(modules/AuroraBots)
add_subdirectory(modules/AuroraSkins)

# --- Aurora tools -----------------------------------------------------
add_subdirectory(src/AuroraTools)

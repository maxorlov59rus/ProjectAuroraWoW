#
# ClasslessSystem module — loadable AzerothCore module
#
# This module provides a "classless" system where players are not restricted
# to a fixed class (e.g., WARRIOR, MAGE). Instead, their abilities are
# determined by a points-based skill system (similar to the existing
# AuroraSkills module) and can be configured per‑character.
#

set(CLASSLESSYSTEM_SOURCES
    src/ClasslessSystemModule.cpp
)

# 1. Add Aurora core layer include directory so this module can include
#    Aurora classes (AuroraSkills, AuroraLogging, etc.).
# 2. Add AzerothCore script mgr include path (for Lua/script hooks)
add_library(ClasslessSystem MODULE ${CLASSLESSYSTEM_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library — CMake forbids target_* on unknown targets.
target_include_directories(ClasslessSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
# Bind against the Aurora core layer and AzerothCore's module API.
target_link_libraries(ClasslessSystem PRIVATE
    AuroraCore
    game
)

set_target_properties(ClasslessSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "ClasslessSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ClasslessSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
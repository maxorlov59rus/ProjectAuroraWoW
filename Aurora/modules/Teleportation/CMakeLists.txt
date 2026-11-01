#
# Teleportation module — loadable AzerothCore module
#
# This module provides a teleportation system for players.
# Players can teleport to various locations, dungeons, and instances.
#

set(TELEPORTATION_SOURCES
    src/TeleportationModule.cpp
)

add_library(Teleportation MODULE ${TELEPORTATION_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library — CMake forbids target_* on unknown targets.
target_include_directories(Teleportation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(Teleportation PRIVATE
    AuroraCore
    game
)

set_target_properties(Teleportation PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "Teleportation"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS Teleportation
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
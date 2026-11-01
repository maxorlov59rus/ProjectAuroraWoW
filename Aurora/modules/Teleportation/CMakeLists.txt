#
# Teleportation module — loadable AzerothCore module
#
# This module provides a teleportation system for players.
# Players can teleport to various locations, dungeons, and instances.
#

set(TELEPORTATION_SOURCES
    src/TeleportationModule.cpp
)

target_include_directories(Teleportation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(Teleportation PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(Teleportation MODULE ${TELEPORTATION_SOURCES})

target_link_libraries(Teleportation PRIVATE
    AuroraCore
    AzerothCoreLib
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
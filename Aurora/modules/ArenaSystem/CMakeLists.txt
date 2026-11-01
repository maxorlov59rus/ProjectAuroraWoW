#
# ArenaSystem module — loadable AzerothCore module
#
# This module provides PvP arena enhancements and ranking systems.
#

set(ARENASYSTEM_SOURCES
    src/ArenaSystemModule.cpp
)

add_library(ArenaSystem MODULE ${ARENASYSTEM_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library — CMake forbids target_* on unknown targets.
target_include_directories(ArenaSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(ArenaSystem PRIVATE
    AuroraCore
    game
)

set_target_properties(ArenaSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "ArenaSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ArenaSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
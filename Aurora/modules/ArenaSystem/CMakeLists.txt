#
# ArenaSystem module — loadable AzerothCore module
#
# This module provides PvP arena enhancements and ranking systems.
#

set(ARENASYSTEM_SOURCES
    src/ArenaSystemModule.cpp
)

target_include_directories(ArenaSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(ArenaSystem PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(ArenaSystem MODULE ${ARENASYSTEM_SOURCES})

target_link_libraries(ArenaSystem PRIVATE
    AuroraCore
    AzerothCoreLib
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
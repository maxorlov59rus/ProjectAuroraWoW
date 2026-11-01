#
# GuildEnhancements module — loadable AzerothCore module
#
# This module provides guild bank, rank, and enhancement features.
#

set(GUILDENHANCEMENTS_SOURCES
    src/GuildEnhancementsModule.cpp
)

target_include_directories(GuildEnhancements PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(GuildEnhancements PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(GuildEnhancements MODULE ${GUILDENHANCEMENTS_SOURCES})

target_link_libraries(GuildEnhancements PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(GuildEnhancements PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "GuildEnhancements"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS GuildEnhancements
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
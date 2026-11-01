#
# AuroraGameModes module — loadable AzerothCore module
#
# This module is a placeholder that demonstrates the standard module
# skeleton. Replace the source with your own game-mode logic.
#

set(AUROGAMEMODES_SOURCES
    AuroraGameModesModule.cpp
)

add_library(AuroraGameModes MODULE ${AUROGAMEMODES_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
target_include_directories(AuroraGameModes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
# AzerothCore core library provides the module API
target_link_libraries(AuroraGameModes PRIVATE
    AuroraCore
    game
)

set_target_properties(AuroraGameModes PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AuroraGameModes"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Install the module to the location the AzerothCore module loader uses
# (see worldserver.conf: modules_directory)
install(TARGETS AuroraGameModes
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)

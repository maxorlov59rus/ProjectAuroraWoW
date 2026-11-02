#
# AuroraSkins module - loadable AzerothCore module
#
# This module is a placeholder that demonstrates the standard module
# skeleton. Replace the source with your own skins logic.
#

set(AURASKINS_SOURCES
    AuroraSkinsModule.cpp
)

add_library(AuroraSkins MODULE ${AURASKINS_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
target_include_directories(AuroraSkins PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(AuroraSkins PRIVATE
    AuroraCore
    game
)

set_target_properties(AuroraSkins PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AuroraSkins"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AuroraSkins
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)

#
# AuroraSkins module — loadable AzerothCore module
#
# This module is a placeholder that demonstrates the standard module
# skeleton. Replace the source with your own skins logic.
#

set(AURASKINS_SOURCES
    AuroraSkinsModule.cpp
)

add_library(AuroraSkins MODULE ${AURASKINS_SOURCES})

target_link_libraries(AuroraSkins PRIVATE
    AuroraCore
    AzerothCoreLib
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

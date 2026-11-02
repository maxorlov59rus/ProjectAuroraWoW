#
# AdminTools module - loadable AzerothCore module
#
# This module provides GM commands and admin panel features.
#

set(ADMINTOOLS_SOURCES
    src/AdminToolsModule.cpp
)

add_library(AdminTools MODULE ${ADMINTOOLS_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(AdminTools PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(AdminTools PRIVATE
    AuroraCore
    game
)

set_target_properties(AdminTools PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AdminTools"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AdminTools
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
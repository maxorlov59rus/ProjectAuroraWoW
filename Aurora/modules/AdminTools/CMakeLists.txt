#
# AdminTools module — loadable AzerothCore module
#
# This module provides GM commands and admin panel features.
#

set(ADMINTOOLS_SOURCES
    src/AdminToolsModule.cpp
)

target_include_directories(AdminTools PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(AdminTools PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(AdminTools MODULE ${ADMINTOOLS_SOURCES})

target_link_libraries(AdminTools PRIVATE
    AuroraCore
    AzerothCoreLib
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
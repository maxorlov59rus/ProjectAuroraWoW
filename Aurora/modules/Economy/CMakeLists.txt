#
# Economy module — loadable AzerothCore module
#
# This module provides auction house enhancements and currency systems.
#

set(ECONOMY_SOURCES
    src/EconomyModule.cpp
)

target_include_directories(Economy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(Economy PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(Economy MODULE ${ECONOMY_SOURCES})

target_link_libraries(Economy PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(Economy PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "Economy"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS Economy
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
#
# LootSystem module — loadable AzerothCore module
#
# This module provides custom loot tables and drop rates.
#

set(LOOTSYSTEM_SOURCES
    src/LootSystemModule.cpp
)

target_include_directories(LootSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(LootSystem PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(LootSystem MODULE ${LOOTSYSTEM_SOURCES})

target_link_libraries(LootSystem PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(LootSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "LootSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS LootSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
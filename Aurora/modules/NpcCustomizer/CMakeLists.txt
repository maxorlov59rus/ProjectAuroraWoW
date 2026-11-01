#
# NpcCustomizer module — loadable AzerothCore module
#
# This module provides custom NPC spawning and configuration.
# Create, modify, and manage NPCs dynamically.
#

set(NPCCUSTOMIZER_SOURCES
    src/NpcCustomizerModule.cpp
)

target_include_directories(NpcCustomizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(NpcCustomizer PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(NpcCustomizer MODULE ${NPCCUSTOMIZER_SOURCES})

target_link_libraries(NpcCustomizer PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(NpcCustomizer PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "NpcCustomizer"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS NpcCustomizer
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
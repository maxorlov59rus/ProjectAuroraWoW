#
# PetSystem module — loadable AzerothCore module
#
# This module provides custom pet management system.
# Manage pet abilities, stats, and behavior.
#

set(PETSYSTEM_SOURCES
    src/PetSystemModule.cpp
)

target_include_directories(PetSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(PetSystem PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(PetSystem MODULE ${PETSYSTEM_SOURCES})

target_link_libraries(PetSystem PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(PetSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "PetSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS PetSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
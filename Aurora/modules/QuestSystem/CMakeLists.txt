#
# QuestSystem module — loadable AzerothCore module
#
# This module provides custom quest creation and management.
#

set(QUESTSYSTEM_SOURCES
    src/QuestSystemModule.cpp
)

target_include_directories(QuestSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(QuestSystem PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(QuestSystem MODULE ${QUESTSYSTEM_SOURCES})

target_link_libraries(QuestSystem PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(QuestSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "QuestSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS QuestSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
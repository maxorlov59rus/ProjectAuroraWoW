#
# QuestSystem module - loadable AzerothCore module
#
# This module provides custom quest creation and management.
#

set(QUESTSYSTEM_SOURCES
    src/QuestSystemModule.cpp
)

add_library(QuestSystem MODULE ${QUESTSYSTEM_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(QuestSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(QuestSystem PRIVATE
    AuroraCore
    game
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
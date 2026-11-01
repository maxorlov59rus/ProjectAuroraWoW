#
# AuroraSkills module - loadable AzerothCore module
#
# This module is a placeholder that demonstrates the standard module
# skeleton. Replace the source with your own skills logic.
#

set(AURASKILLS_SOURCES
    AuroraSkillsModule.cpp
)

add_library(AuroraSkills MODULE ${AURASKILLS_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
target_include_directories(AuroraSkills PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(AuroraSkills PRIVATE
    AuroraCore
    game
)

set_target_properties(AuroraSkills PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AuroraSkills"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AuroraSkills
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)

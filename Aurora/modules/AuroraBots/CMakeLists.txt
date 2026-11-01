#
# AuroraBots module - loadable AzerothCore module
#
# This module provides a bot management system with Lua-script support.
# It registers commands !createbot, !delbot, !listbots and !botai.
# Bots are stored in the database and can have custom AI via Lua scripts.
#

set(AURABOTS_SOURCES
    src/AuroraBotsModule.cpp
)

# 1. Add Aurora core layer include directory so this module can include
#    Aurora classes (AuroraBot, AuroraLogging, etc.).
# 2. Add the AzerothCore script mgr include path (where Lua scripting is handled)
#    We assume AzerothCore provides ScriptMgr; the exact path is set by the
#    master CMake when AzerothCore is added as a subdirectory.
add_library(AuroraBots MODULE ${AURABOTS_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(AuroraBots PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
# Bind against the Aurora core layer and AzerothCore's module/API.
target_link_libraries(AuroraBots PRIVATE
    AuroraCore
    game
    # Link against AzerothCore's script module if needed
)

set_target_properties(AuroraBots PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AuroraBots"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AuroraBots
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
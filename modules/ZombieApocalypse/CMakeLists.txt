#
# ZombieApocalypse module - loadable AzerothCore module
#
# This module introduces a zombie apocalypse event system where zombies
# spawn in random locations across the game world. Players can fight
# hordes of zombies, complete objectives, and survive as long as possible.
#

set(ZOMBIEAPP_SOURCES
    src/ZombieApocalypseModule.cpp
)

# 1. Add Aurora core layer include directory so this module can include
#    Aurora classes (AuroraBot, AuroraLogging, etc.).
# 2. Add AzerothCore script mgr include path (where Lua scripting is handled)
add_library(ZombieApocalypse MODULE ${ZOMBIEAPP_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(ZombieApocalypse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
# Bind against the Aurora core layer and AzerothCore's module API.
target_link_libraries(ZombieApocalypse PRIVATE
    AuroraCore
    game
)

set_target_properties(ZombieApocalypse PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "ZombieApocalypse"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ZombieApocalypse
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
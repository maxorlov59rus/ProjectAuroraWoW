#
# ZombieApocalypse module — loadable AzerothCore module
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
target_include_directories(ZombieApocalypse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

# 2. Add AzerothCore script mgr include path (where Lua scripting is handled)
target_include_directories(ZombieApocalypse PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(ZombieApocalypse MODULE ${ZOMBIEAPP_SOURCES})

# Bind against the Aurora core layer and AzerothCore's module API.
target_link_libraries(ZombieApocalypse PRIVATE
    AuroraCore
    AzerothCoreLib
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
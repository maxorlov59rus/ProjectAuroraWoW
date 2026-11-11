#
# WeatherSystem module - loadable AzerothCore module
#
# This module provides dynamic weather affecting gameplay.
#

set(WEATHERSYSTEM_SOURCES
    src/WeatherSystemModule.cpp
)

add_library(WeatherSystem MODULE ${WEATHERSYSTEM_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(WeatherSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(WeatherSystem PRIVATE
    AuroraCore
    game
)

set_target_properties(WeatherSystem PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "WeatherSystem"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS WeatherSystem
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
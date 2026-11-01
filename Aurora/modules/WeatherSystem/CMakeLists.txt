#
# WeatherSystem module — loadable AzerothCore module
#
# This module provides dynamic weather affecting gameplay.
#

set(WEATHERSYSTEM_SOURCES
    src/WeatherSystemModule.cpp
)

target_include_directories(WeatherSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(WeatherSystem PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(WeatherSystem MODULE ${WEATHERSYSTEM_SOURCES})

target_link_libraries(WeatherSystem PRIVATE
    AuroraCore
    AzerothCoreLib
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
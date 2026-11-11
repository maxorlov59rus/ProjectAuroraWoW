#
# AuroraCore - Aurora custom C++ layer
#
# Builds the Aurora custom layer that hooks into AzerothCore. This library
# is linked by the Aurora modules (AuroraSkills, AuroraSkins,
# AuroraGameModes) and by the server launcher.
#
# Notes:
#  - AuroraCore deliberately does not depend on AzerothCoreLib directly;
#    it only needs C++17 and the project-level header locations. The
#    consumers (modules, launcher) add AzerothCoreLib as needed.
#

set(AURORA_CORE_SOURCES
    AuroraPCH.cpp
    AuroraLogging.cpp
    AuroraSkills.cpp
    AuroraSkins.cpp
    AuroraInterface.cpp
)

add_library(AuroraCore STATIC ${AURORA_CORE_SOURCES})

target_include_directories(AuroraCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Keep C++17 consistent with AzerothCore's top-level configuration.
set_target_properties(AuroraCore PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AuroraCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

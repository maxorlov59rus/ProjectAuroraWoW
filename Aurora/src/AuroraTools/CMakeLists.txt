#
# AuroraTools — database and world-config utilities
#

set(AURATOOLS_SOURCES
    AuroraTools.cpp
)

add_library(AuroraTools STATIC ${AURATOOLS_SOURCES})

target_include_directories(AuroraTools PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# AuroraTools consumes the Aurora core layer (AuroraLogging) and the
# project-level headers. Provide the include path when linking.
target_link_libraries(AuroraTools PRIVATE
    AuroraCore
)

set_target_properties(AuroraTools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS AuroraTools
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

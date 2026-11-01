#
# CampaignOfDarkness module — loadable AzerothCore module
#
# This module introduces a "Campaign of Darkness" system with:
# - NPCs with custom dialogues
# - Spawn lists for various locations
# - Quest management
# - Boss encounters
# - Custom shops
#

set(CAMPAIGNOFDARKNESS_SOURCES
    src/CampaignOfDarknessModule.cpp
)

target_include_directories(CampaignOfDarkness PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(CampaignOfDarkness PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(CampaignOfDarkness MODULE ${CAMPAIGNOFDARKNESS_SOURCES})

target_link_libraries(CampaignOfDarkness PRIVATE
    AuroraCore
    AzerothCoreLib
)

set_target_properties(CampaignOfDarkness PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "CampaignOfDarkness"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS CampaignOfDarkness
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)
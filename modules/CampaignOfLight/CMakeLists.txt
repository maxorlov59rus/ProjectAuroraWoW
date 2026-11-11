#
# CampaignOfLight module - loadable AzerothCore module
#
# Alliance campaign content: NPCs, dialogues, quests, bosses, shops.
# Counterpart to CampaignOfDarkness (Horde campaign).
#

set(CAMPAIGNOFLIGHT_SOURCES
    src/CampaignOfLightModule.cpp
)

add_library(CampaignOfLight MODULE ${CAMPAIGNOFLIGHT_SOURCES})


# Aurora core layer headers (AuroraPCH.h, AuroraLogging.h, ...).
# Must come AFTER add_library - CMake forbids target_* on unknown targets.
target_include_directories(CampaignOfLight PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)
target_link_libraries(CampaignOfLight PRIVATE
    AuroraCore
    game
)

set_target_properties(CampaignOfLight PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "CampaignOfLight"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS CampaignOfLight
    LIBRARY DESTINATION "${AURORA_MODULE_DIR}"
)

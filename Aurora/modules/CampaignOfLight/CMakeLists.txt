/*
 * CampaignOfLight module — loadable AzerothCore module
 *
 * Alliance campaign content: NPCs, dialogues, quests, bosses, shops.
 * Counterpart to CampaignOfDarkness (Horde campaign).
 */

set(CAMPAIGNOFLIGHT_SOURCES
    src/CampaignOfLightModule.cpp
)

target_include_directories(CampaignOfLight PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/Aurora
)

target_include_directories(CampaignOfLight PRIVATE
    ${AURORA_CORE_INCLUDE_DIR}
)

add_library(CampaignOfLight MODULE ${CAMPAIGNOFLIGHT_SOURCES})

target_link_libraries(CampaignOfLight PRIVATE
    AuroraCore
    AzerothCoreLib
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

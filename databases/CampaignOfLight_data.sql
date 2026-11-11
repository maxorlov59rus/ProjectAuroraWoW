-- ============================================================================
# Campaign of Light — NPC Data
# Alliance campaign NPCs (Light faction)
# Entry IDs: 93000–93999
-- ============================================================================

INSERT IGNORE INTO campaign_npcs (entry, name, title, faction, level, health, float_x, float_y, float_z, float_o, map_id, dialogues_script, is_boss, is_vendor) VALUES
-- Stormwind leaders
(93001, 'King Varian Wrynn', 'King of Stormwind', 'alliance', 80, 80000, -6200, -2700, 130, 0, 0, 'varian_light_dialogues', 0, 0),
(93002, 'Lady Jaina Proudmoore', 'Archmage of Dalaran', 'alliance', 80, 60000, -6100, -2600, 130, 0, 0, 'jaina_light_dialogues', 0, 0),
(93003, 'Archbishop Benedict', 'Leader of the Silver Hand', 'alliance', 75, 50000, -6150, -2650, 130, 0, 0, 'benedict_light_dialogues', 0, 0),

-- Stormwind merchants
(93010, 'Garrick', 'Weapon Master', 'alliance', 50, 25000, -6250, -2750, 130, 0, 0, '', 0, 1),
(93011, 'Lillian Dawnstride', 'Enchanting Master', 'alliance', 50, 25000, -6280, -2780, 130, 0, 0, '', 0, 1),
(93012, 'Brom Ar'gor', 'Armor Smith', 'alliance', 45, 22000, -6300, -2800, 130, 0, 0, '', 0, 1),

-- Ironforge leaders
(93020, 'King Magni Bronzebeard', 'King of Ironforge', 'alliance', 80, 70000, -4800, -2800, 310, 0, 0, 'magni_light_dialogues', 0, 0),
(93021, 'Moira Thaurissan', 'Queen of Dun Morogh', 'alliance', 70, 45000, -4700, -2900, 310, 0, 0, 'moira_light_dialogues', 0, 0),

-- Exodar leaders
(93030, 'Velen', 'Prophet of the Draenei', 'alliance', 80, 90000, 3000, -1200, 100, 0, 530, 'velen_light_dialogues', 0, 0),
(93031, 'A'dal', 'Light Avatar', 'alliance', 80, 100000, 3100, -1100, 150, 0, 530, 'adal_light_dialogues', 1, 0),

-- Neutral Light bosses
(93050, 'High Inquisitor Whitemane', 'Undead Inquisitor', 'neutral', 80, 120000, -5600, -200, 40, 0, 1, 'whitemane_light_dialogues', 1, 0),
(93051, 'Archbishop Bonham', 'Fallen Archbishop', 'neutral', 75, 100000, -5500, -100, 40, 0, 1, 'bonham_light_dialogues', 1, 0);

-- ============================================================================
# Campaign of Light — NPC Data (Horde)
# Entry IDs: 93100–93199
-- ============================================================================

INSERT IGNORE INTO campaign_npcs (entry, name, title, faction, level, health, float_x, float_y, float_z, float_o, map_id, dialogues_script, is_boss, is_vendor) VALUES
-- Horde Light-aligned NPCs (converts from Darkness)
(93101, 'Baine Bloodhoof', 'Son of Thrall', 'horde', 80, 65000, 1400, -6500, 60, 0, 1, 'baine_light_dialogues', 0, 0),
(93102, 'High Priestess Elloia', 'Light Champion of Horde', 'horde', 75, 50000, 1500, -6400, 60, 0, 1, 'elloia_light_dialogues', 0, 0),

-- Horde merchants
(93110, 'Grom''gol', 'Weapon Vendor', 'horde', 45, 22000, 1450, -6550, 60, 0, 1, '', 0, 1),
(93111, 'Mikani Moonshadow', 'Enchanting Vendor', 'horde', 45, 22000, 1480, -6580, 60, 0, 1, '', 0, 1);

-- ============================================================================
# Campaign of Light — Quest Data
# Entry IDs: 30000–30999 (Alliance), 31000–31999 (Horde)
-- ============================================================================

INSERT IGNORE INTO campaign_quests (quest_id, title, description, faction, level, type, reward_gold, reward_xp, start_npc_entry, end_npc_entry) VALUES
-- Alliance Light campaign quests
(30001, 'The Light Calls', 'Seek out Archbishop Benedict for guidance.', 'alliance', 20, 'main', 5000, 8000, 93001, 93003),
(30002, 'Shadows in the Cathedral', 'Investigate the disturbances in Stormwind Cathedral.', 'alliance', 25, 'main', 8000, 12000, 93003, 93003),
(30003, 'The Silver Hand Reborn', 'Gather relics for the Silver Hand order.', 'alliance', 35, 'main', 15000, 20000, 93003, 93002),
(30004, 'Jaina\'s Request', 'Assist Lady Jaina in Dalaran.', 'alliance', 40, 'side', 10000, 15000, 93002, 93002),
(30005, 'The Dalaran Siege', 'Defend Dalaran from the enemy forces.', 'alliance', 55, 'main', 25000, 30000, 93002, 93002),
(30006, 'Whitemane\'s Fall', 'Help High Inquisitor Whitemane regain her honor.', 'alliance', 60, 'boss', 50000, 50000, 93001, 93050),
(30007, 'Bonham\'s Judgment', 'Face the fallen Archbishop Bonham.', 'alliance', 70, 'boss', 75000, 75000, 93002, 93051),
(30008, 'The Light\'s Triumph', 'Complete the Campaign of Light.', 'alliance', 80, 'main', 100000, 100000, 93001, 93001),

-- Horde Light campaign quests (converts from Darkness)
(31001, 'The Light Within', 'Discover the power of the Light within yourself.', 'horde', 20, 'main', 5000, 8000, 93101, 93102),
(31002, 'Shadows of the Past', 'Confront the darkness that plagues the Horde.', 'horde', 30, 'main', 12000, 18000, 93102, 93102),
(31003, 'Blood and Light', 'Prove that the Horde can embrace the Light.', 'horde', 50, 'main', 20000, 30000, 93101, 93102),
(31004, 'The Sunwell Renewed', 'Help restore the power of the Light to the Horde.', 'horde', 70, 'main', 50000, 60000, 93102, 93102),
(31005, 'A New Dawn for the Horde', 'Complete the Campaign of Light as the Horde.', 'horde', 80, 'main', 100000, 100000, 93101, 93101);

-- ============================================================================
# Campaign of Light — Boss Data
# Entry IDs: 95000–95999
-- ============================================================================

INSERT IGNORE INTO campaign_bosses (entry, name, title, faction, level, health, mana, abilities, float_x, float_y, float_z, map_id, respawn_time) VALUES
-- Alliance bosses
(95001, 'High Inquisitor Whitemane', 'Fallen Champion', 'neutral', 80, 200000, 100000, 'smite_shadow, holy_shield', -5600, -200, 40, 1, 3600),
(95002, 'Archbishop Bonham', 'The Corrupted Light', 'neutral', 75, 180000, 80000, 'light_burst, holy_wrath', -5500, -100, 40, 1, 3600),

-- Draenei bosses
(95010, 'Kara Dawnstrider', 'The Exiled', 'neutral', 80, 150000, 60000, 'naaru_light, holy_aura', 3200, -1300, 100, 530, 1800),

-- Neutral Light bosses
(95020, 'The Light Avatar', 'Avatar of Pure Light', 'neutral', 80, 500000, 200000, 'divine_fury, holy_bolt_storm, resurrection', 3100, -1100, 150, 530, 7200);

-- ============================================================================
# Campaign of Light — Shop Data
-- ============================================================================

INSERT IGNORE INTO campaign_shops (vendor_entry, name, faction, items, discount_alliance) VALUES
(94001, 'Light\'s Blessing Armory', 'alliance', '1,2,3,4,5,6,7,8,9,10', 0.1),
(94002, 'Dalaran Enchantments', 'alliance', '11,12,13,14,15', 0.15),
(94003, 'Stormwind General Goods', 'alliance', '16,17,18,19,20', 0.05);

INSERT IGNORE INTO campaign_shops (vendor_entry, name, faction, items, discount_horde) VALUES
(94010, 'Horde Light Merchant', 'horde', '1,2,3,4,5', 0.1);

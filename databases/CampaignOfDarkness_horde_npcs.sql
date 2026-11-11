-- Insert Horde NPCs
INSERT IGNORE INTO campaign_npcs (entry, name, title, faction, level, health, float_x, float_y, float_z, float_o, map_id, dialogues_script, is_boss, is_vendor) VALUES
(90001, 'Thrall', 'Warchief of the Horde', 'horde', 80, 50000, -5200, -1000, 50, 0, 1, 'thrall_dialogues', 0, 0),
(90002, 'Garrosh Hellscream', 'Overlord', 'horde', 80, 40000, -5100, -900, 50, 0, 1, 'garrosh_dialogues', 0, 0),
(90003, 'Sylvanas Windrunner', 'Banshee Queen', 'horde', 80, 45000, -5000, -800, 50, 0, 1, 'sylvanas_dialogues', 0, 0),
(90004, 'Vol\'jin', 'Sen\'jin Leader', 'horde', 75, 35000, -5150, -950, 50, 0, 1, 'voljin_dialogues', 0, 0),
(90005, 'Kaz the Scream', 'Weapon Vendor', 'horde', 40, 20000, -5250, -980, 50, 0, 1, '', 0, 1),
(90006, 'Mok\'tar', 'Alchemy Vendor', 'horde', 40, 20000, -5280, -1010, 50, 0, 1, '', 0, 1),
(90010, 'Blackrock Overlord', 'Blackrock Boss', 'neutral', 80, 100000, -3400, -2800, 120, 0, 1, 'blackrock_dialogues', 1, 0);

-- Insert Alliance NPCs
INSERT IGNORE INTO campaign_npcs (entry, name, title, faction, level, health, float_x, float_y, float_z, float_o, map_id, dialogues_script, is_boss, is_vendor) VALUES
(91001, 'King Magni Bronzebeard', 'King of Ironforge', 'alliance', 80, 50000, -6000, -2800, 500, 0, 0, 'magni_dialogues', 0, 0),
(91002, 'King Varian Wrynn', 'King of Stormwind', 'alliance', 80, 50000, -5900, -2700, 500, 0, 0, 'varian_dialogues', 0, 0),
(91003, 'Mekkatorque', 'Gnome Leader', 'alliance', 75, 35000, -5950, -2750, 500, 0, 1, 'mekkatorque_dialogues', 0, 0),
(91004, 'Jaina Proudmoore', 'Lord Admiral', 'alliance', 80, 45000, -5800, -2600, 500, 0, 1, 'jaina_dialogues', 0, 0),
(91005, 'Barrett Coppercrank', 'Weapon Vendor', 'alliance', 40, 20000, -5850, -2780, 500, 0, 0, '', 0, 1),
(91006, 'Harna Goldenrod', 'Alchemy Vendor', 'alliance', 40, 20000, -5820, -2740, 500, 0, 0, '', 0, 1),
(91010, 'Ironforge Guard', 'Ironforge Boss', 'neutral', 80, 100000, -6100, -2900, 500, 0, 0, 'ironforge_guard_dialogues', 1, 0);
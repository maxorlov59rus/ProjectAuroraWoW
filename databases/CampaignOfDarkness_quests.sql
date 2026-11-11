-- Insert Quests
INSERT IGNORE INTO campaign_quests (quest_id, title, description, faction, level, type, reward_gold, reward_xp, start_npc_entry, end_npc_entry) VALUES
(10001, 'The Darkness Approaches', 'Investigate Durotar disturbances.', 'horde', 20, 'main', 5000, 5000, 90001, 90001),
(10002, 'Blackrock Menace', 'Slay the Blackrock Overlord.', 'horde', 40, 'side', 10000, 10000, 90002, 90001),
(10003, 'Zul\'Gurub Corruption', 'Cleanse Zul\'Gurub corruption.', 'horde', 60, 'side', 15000, 15000, 90003, 90003),
(10004, 'Shadows in Azshara', 'Defeat Darkshore Guardian.', 'horde', 50, 'side', 12000, 12000, 90001, 90001),
(20001, 'The Ironforge Threat', 'Investigate Ironforge disturbances.', 'alliance', 20, 'main', 5000, 5000, 91001, 91001),
(20002, 'Sentinel Menace', 'Slay Ironforge Sentinel.', 'alliance', 40, 'side', 10000, 10000, 91002, 91001),
(20003, 'Felwood Corruption', 'Investigate Felwood corruption.', 'alliance', 60, 'side', 15000, 15000, 91004, 91004),
(20004, 'Darkshore Guardian', 'Defeat Darkshore Guardian.', 'alliance', 50, 'side', 12000, 12000, 91001, 91001);
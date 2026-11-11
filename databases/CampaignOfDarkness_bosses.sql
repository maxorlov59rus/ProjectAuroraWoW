-- Insert Bosses
INSERT IGNORE INTO campaign_bosses (entry, name, title, faction, level, health, float_x, float_y, float_z, float_o, map_id, spawn_script, is_active) VALUES
(92001, 'Blackrock Overlord', 'Blackrock Boss', 'neutral', 80, 100000, -3400, -2800, 120, 0, 1, 'blackrock_spawn', 1),
(92002, 'Darkshore Guardian', 'Darkshore Boss', 'neutral', 80, 120000, -4200, 1600, 100, 0, 1, 'darkshore_spawn', 1),
(92003, 'Ironforge Sentinel', 'Ironforge Boss', 'neutral', 80, 110000, -6100, -2900, 500, 0, 0, 'ironforge_spawn', 1),
(92004, 'Durotar Warlord', 'Durotar Boss', 'neutral', 80, 95000, -5200, -1000, 50, 0, 1, 'durotar_spawn', 1);

-- Insert Shops
INSERT IGNORE INTO campaign_shops (vendor_entry, name, faction, items, lua_script, discount_horde, discount_alliance) VALUES
(90005, 'Kaz Weapon Shop', 'horde', '12345,12346,12347', 'weapon_shop_script', 0.10, 0.00),
(90006, 'Mok\'tar Alchemy Shop', 'horde', '23456,23457,23458', 'alchemy_shop_script', 0.10, 0.00),
(91005, 'Barrett Weapon Shop', 'alliance', '23456,23457,23458', 'weapon_shop_script', 0.00, 0.10),
(91006, 'Harna Alchemy Shop', 'alliance', '34567,34568,34569', 'alchemy_shop_script', 0.00, 0.10);
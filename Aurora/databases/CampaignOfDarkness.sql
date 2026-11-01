-- Campaign of Darkness Database Schema
-- Tables for NPCs, dialogues, quests, bosses, shops, and spawn lists

-- Campaign NPCs table
CREATE TABLE IF NOT EXISTS campaign_npcs (
    entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    title VARCHAR(64) DEFAULT '',
    level INT NOT NULL DEFAULT 1,
    faction INT NOT NULL DEFAULT 0,
    spawn_x FLOAT NOT NULL DEFAULT 0,
    spawn_y FLOAT NOT NULL DEFAULT 0,
    spawn_z FLOAT NOT NULL DEFAULT 0,
    map_id INT NOT NULL DEFAULT 0,
    spawn_time INT NOT NULL DEFAULT 0,
    dialogues TEXT,
    is_boss BOOLEAN DEFAULT FALSE,
    is_vendor BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Campaign Quests table
CREATE TABLE IF NOT EXISTS campaign_quests (
    quest_id INT PRIMARY KEY,
    title VARCHAR(128) NOT NULL,
    description TEXT,
    level INT NOT NULL DEFAULT 1,
    reward_gold INT NOT NULL DEFAULT 0,
    lua_script VARCHAR(128) DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Campaign Bosses table
CREATE TABLE IF NOT EXISTS campaign_bosses (
    entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    level INT NOT NULL DEFAULT 1,
    health INT NOT NULL DEFAULT 1000,
    abilities TEXT,
    spawn_x FLOAT NOT NULL DEFAULT 0,
    spawn_y FLOAT NOT NULL DEFAULT 0,
    spawn_z FLOAT NOT NULL DEFAULT 0,
    map_id INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Campaign Shops table
CREATE TABLE IF NOT EXISTS campaign_shops (
    vendor_entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    items TEXT,
    lua_script VARCHAR(128) DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Campaign Spawn List table
CREATE TABLE IF NOT EXISTS campaign_spawn_list (
    id INT AUTO_INCREMENT PRIMARY KEY,
    spawn_type VARCHAR(32) NOT NULL, -- 'npc', 'boss', 'vendor'
    entry INT NOT NULL,
    x FLOAT NOT NULL,
    y FLOAT NOT NULL,
    z FLOAT NOT NULL,
    map_id INT NOT NULL DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Campaign Dialogue Nodes table
CREATE TABLE IF NOT EXISTS campaign_dialogues (
    id INT AUTO_INCREMENT PRIMARY KEY,
    npc_entry INT NOT NULL,
    node_id INT NOT NULL,
    text TEXT NOT NULL,
    next_nodes VARCHAR(256) DEFAULT '',
    lua_script VARCHAR(128) DEFAULT '',
    is_end BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;
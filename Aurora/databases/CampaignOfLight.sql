-- ============================================================================
# Campaign of Light Database Schema
# Tables for Alliance campaign content
# ============================================================================

-- NPC Table (same schema as CampaignOfDarkness)
CREATE TABLE IF NOT EXISTS campaign_npcs (
    entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    title VARCHAR(64) DEFAULT '',
    faction ENUM('horde','alliance','neutral') NOT NULL DEFAULT 'neutral',
    level INT NOT NULL DEFAULT 1,
    health INT NOT NULL DEFAULT 1000,
    mana INT NOT NULL DEFAULT 1000,
    faction_template INT NOT NULL DEFAULT 0,
    float_x FLOAT NOT NULL DEFAULT 0,
    float_y FLOAT NOT NULL DEFAULT 0,
    float_z FLOAT NOT NULL DEFAULT 0,
    float_o FLOAT NOT NULL DEFAULT 0,
    map_id INT NOT NULL DEFAULT 0,
    spawn_time INT NOT NULL DEFAULT 0,
    dialogues_script VARCHAR(128) DEFAULT '',
    is_boss BOOLEAN DEFAULT FALSE,
    is_vendor BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Dialogue Nodes Table
CREATE TABLE IF NOT EXISTS campaign_dialogues (
    id INT AUTO_INCREMENT PRIMARY KEY,
    npc_entry INT NOT NULL,
    node_id INT NOT NULL,
    speaker_name VARCHAR(64) DEFAULT 'NPC',
    text TEXT NOT NULL,
    text_faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    next_nodes VARCHAR(256) DEFAULT '',
    lua_script VARCHAR(128) DEFAULT '',
    is_end BOOLEAN DEFAULT FALSE,
    is_start BOOLEAN DEFAULT FALSE,
    required_quest INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (npc_entry) REFERENCES campaign_npcs(entry)
) ENGINE=InnoDB;

-- Dialogue Options Table
CREATE TABLE IF NOT EXISTS campaign_dialogue_options (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dialogue_id INT NOT NULL,
    option_text VARCHAR(256) NOT NULL,
    next_node_id INT DEFAULT -1,
    lua_script VARCHAR(128) DEFAULT '',
    required_faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    required_level INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (dialogue_id) REFERENCES campaign_dialogues(id)
) ENGINE=InnoDB;

-- Quests Table
CREATE TABLE IF NOT EXISTS campaign_quests (
    quest_id INT PRIMARY KEY,
    title VARCHAR(128) NOT NULL,
    description TEXT,
    faction ENUM('horde','alliance','neutral') NOT NULL DEFAULT 'neutral',
    level INT NOT NULL DEFAULT 1,
    type ENUM('daily','weekly','main','side','boss') DEFAULT 'side',
    reward_gold INT NOT NULL DEFAULT 0,
    reward_xp INT NOT NULL DEFAULT 0,
    reward_items TEXT DEFAULT '',
    lua_script VARCHAR(128) DEFAULT '',
    start_npc_entry INT DEFAULT 0,
    end_npc_entry INT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Bosses Table
CREATE TABLE IF NOT EXISTS campaign_bosses (
    entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    title VARCHAR(64) DEFAULT '',
    faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    level INT NOT NULL DEFAULT 80,
    health INT NOT NULL DEFAULT 100000,
    mana INT NOT NULL DEFAULT 50000,
    abilities TEXT,
    spawn_script VARCHAR(128) DEFAULT '',
    float_x FLOAT NOT NULL DEFAULT 0,
    float_y FLOAT NOT NULL DEFAULT 0,
    float_z FLOAT NOT NULL DEFAULT 0,
    float_o FLOAT NOT NULL DEFAULT 0,
    map_id INT NOT NULL DEFAULT 0,
    spawn_time INT NOT NULL DEFAULT 0,
    respawn_time INT NOT NULL DEFAULT 3600,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Shops Table
CREATE TABLE IF NOT EXISTS campaign_shops (
    vendor_entry INT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    items TEXT,
    lua_script VARCHAR(128) DEFAULT '',
    discount_horde FLOAT DEFAULT 0,
    discount_alliance FLOAT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Spawn List Table
CREATE TABLE IF NOT EXISTS campaign_spawn_list (
    id INT AUTO_INCREMENT PRIMARY KEY,
    spawn_type VARCHAR(32) NOT NULL DEFAULT 'npc',
    entry INT NOT NULL,
    faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    zone VARCHAR(64) DEFAULT '',
    float_x FLOAT NOT NULL DEFAULT 0,
    float_y FLOAT NOT NULL DEFAULT 0,
    float_z FLOAT NOT NULL DEFAULT 0,
    float_o FLOAT NOT NULL DEFAULT 0,
    map_id INT NOT NULL DEFAULT 0,
    spawn_time INT NOT NULL DEFAULT 0,
    respawn_time INT NOT NULL DEFAULT 600,
    max_count INT NOT NULL DEFAULT 1,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Player Campaign Progress Table
CREATE TABLE IF NOT EXISTS campaign_player_progress (
    guid BIGINT NOT NULL,
    campaign_name VARCHAR(64) NOT NULL DEFAULT 'Campaign of Darkness',
    faction ENUM('horde','alliance','neutral') DEFAULT 'neutral',
    completed_quests TEXT DEFAULT '',
    killed_bosses TEXT DEFAULT '',
    visited_locations TEXT DEFAULT '',
    current_quest INT DEFAULT 0,
    reputation INT DEFAULT 0,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (guid, campaign_name)
) ENGINE=InnoDB;

-- Classless System Database Schema
-- Table for classless players (not restricted to fixed classes like WARRIOR, MAGE)
-- Fields: guid (linked to characters table), name, level, skill points, lua script reference, status
-- This table can be linked to the characters table via guid or used independently for custom bot/player systems

CREATE TABLE IF NOT EXISTS classless_players (
    guid BIGINT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    level INT NOT NULL DEFAULT 1,
    skill_points INT NOT NULL DEFAULT 0,
    lua_script VARCHAR(128) DEFAULT '',
    is_running BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY idx_name (name)
) ENGINE=InnoDB;
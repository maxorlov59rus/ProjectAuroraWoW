-- Aurora Database Initialization Script
-- Run this script after setting up MySQL to create the necessary tables for ClasslessSystem and AuroraBots

-- ============================================
-- Classless System Tables
-- ============================================

-- Create classless_players table if not exists
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

-- ============================================
-- Aurora Bots Tables
-- ============================================

-- Create bots table if not exists
CREATE TABLE IF NOT EXISTS bots (
    guid BIGINT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    class VARCHAR(32) DEFAULT 'NONE',
    level INT NOT NULL DEFAULT 1,
    player_guid BIGINT DEFAULT NULL,
    lua_script VARCHAR(128) DEFAULT '',
    is_running BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Create bot_scripts table if not exists
CREATE TABLE IF NOT EXISTS bot_scripts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    script_name VARCHAR(64) NOT NULL UNIQUE,
    lua_code TEXT NOT NULL,
    version VARCHAR(32) DEFAULT '1.0',
    author VARCHAR(64) DEFAULT 'Aurora',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- ============================================
-- Initial Data (Optional)
-- ============================================

-- Example: Add a default classless player template
-- INSERT IGNORE INTO classless_players (guid, name, level, skill_points, lua_script)
-- VALUES (0, 'DefaultClassless', 1, 0, '');

-- Example: Add a default bot script template
-- INSERT IGNORE INTO bot_scripts (script_name, lua_code, version, author)
-- VALUES ('patrol', '-- Example patrol script\n--\n-- When bot is created:\n-- function OnBotCreate(botGuid) end\n--\n-- When bot updates every tick:\n-- function OnBotUpdate(botGuid, diff_ms) end\n', '1.0', 'Aurora');

-- ============================================
-- Index Optimization (Optional)
-- ============================================

-- Add indexes for faster lookups
CREATE INDEX idx_classless_name ON classless_players(name);
CREATE INDEX idx_bots_guid ON bots(guid);
CREATE INDEX idx_bots_player_guid ON bots(player_guid);
CREATE INDEX idx_bots_is_running ON bots(is_running);
CREATE INDEX idx_bots_name ON bots(name);

-- ============================================
-- Script Completion Message
-- ============================================

SELECT 'Aurora Database Initialization Complete!' AS status;
SELECT 'Tables created: classless_players, bots, bot_scripts' AS tables_created;
SELECT 'Indexes created for optimization' AS indexes_created;
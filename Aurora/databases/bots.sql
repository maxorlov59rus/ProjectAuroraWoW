-- Aurora Bots Database Schema
-- Table for bot information and status
-- Fields: guid, name, class, level, associated player guid, lua script, status

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

-- Table for bot Lua scripts storage
-- Fields: script name, script content, version, author

CREATE TABLE IF NOT EXISTS bot_scripts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    script_name VARCHAR(64) NOT NULL UNIQUE,
    lua_code TEXT NOT NULL,
    version VARCHAR(32) DEFAULT '1.0',
    author VARCHAR(64) DEFAULT 'Aurora',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;
/**
 * AuroraTools — database loading and world-config utilities
 *
 * Lightweight helpers for Aurora:
 *  - DatabaseManager: open/close/seed the Aurora database.
 *  - WorldIniReader: read the world configuration file and expose values
 *    as typed keys.
 *
 * These are intentionally simple so that the Aurora layer does not
 * introduce heavy third-party dependencies. The actual DB schema lives in
 * Aurora/databases/ and is loaded by AzerothCore's own database loader.
 */
#pragma once

#include <AuroraPCH.h>

#include <map>
#include <optional>
#include <string>

namespace Aurora {

namespace DatabaseManager {

/**
 * Connect to the Aurora database. Returns the connection token on success.
 * In a real port this uses the MySQL client library and the credentials
 * stored in Aurora's config file.
 */
bool Initialize();

/**
 * Shutdown the database connection, if any.
 */
void Shutdown();

/**
 * Seed default data. Return true when successful (or already seeded).
 * In a full port this checks the DB schema version and runs the SQL from
 * Aurora/databases/.
 */
bool SeedDefaults();

/**
 * Return the number of characters currently registered in the DB.
 * Implementation is intentionally minimal.
 */
uint32_t GetCharacterCount();

} // namespace DatabaseManager

namespace WorldIniReader {

/**
 * Read a key-value configuration file. Lines are of the form:
 *   key = value
 *   ; comment
 * Empty lines and comments are ignored.
 */
bool Load(const std::string& path, std::map<std::string, std::string>& out);

/**
 * Helper to look up a value. Returns std::nullopt when the key is missing.
 */
std::optional<std::string> Get(const std::map<std::string, std::string>& cfg,
                               const std::string& key);

/**
 * Print the whole configuration for diagnostics.
 */
void Print(const std::map<std::string, std::string>& cfg);

} // namespace WorldIniReader

} // namespace Aurora

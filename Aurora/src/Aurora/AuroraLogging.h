/**
 * AuroraLogging
 *
 * Central wrapper for Aurora's diagnostics. Delegates to AzerothCore's
 * logging framework, adding Aurora-specific prefixes and formatting.
 *
 * Usage:
 *   Aurora::Logging::Info("world", "world ready");
 *   Aurora::Logging::Warn("skills", "skill table not loaded");
 *   Aurora::Logging::Error("db", "unable to load characters: %d", err);
 *
 * Note:
 *  - Do not use for every packet. Logging is for diagnostics, not for
 *    per-player chat. Keep the log level configurable.
 */
#pragma once

#include <string>

namespace Aurora {

enum class Level
{
    Debug,   // detailed diagnostics
    Info,    // operational health checks
    Warn,    // recoverable problems
    Error,   // unexpected failures
    Off      // logging disabled
};

class Logging
{
public:
    // Log a message with the given level and source.
    static void Log(Level level, const std::string& source, const std::string& msg);

    // Convenience overloads
    static void Debug(const std::string& source, const std::string& msg);
    static void Info(const std::string& source, const std::string& msg);
    static void Warn(const std::string& source, const std::string& msg);
    static void Error(const std::string& source, const std::string& msg);

    // Return the current effective log level (read from Aurora config).
    static Level GetLevel();

    // Set the global log level. Normally called from Aurora::Initialize().
    static void SetLevel(Level level);

private:
    static Level s_level;
};

} // namespace Aurora

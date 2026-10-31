/**
 * AuroraLogging.cpp — central Aurora diagnostics.
 *
 * Lightweight wrapper that writes to stdout. In a production build this
 * should be replaced with the AzerothCore logging framework (AzerothCore
 * uses spdlog via the deps). The level is configurable at runtime from
 * Aurora's config (see docs/AURORA_CUSTOMIZATION.md).
 */
#include <AuroraPCH.h>

#include <AuroraLogging.h>
#include <cstdio>

namespace Aurora {

Level Logging::s_level = Level::Info;

void Logging::Log(Level level, const std::string& source, const std::string& msg)
{
    if (level < s_level) {
        return;
    }
    std::printf("[Aurora/%s] %s\n", source.c_str(), msg.c_str());
    std::fflush(stdout);
}

void Logging::Debug(const std::string& source, const std::string& msg)
{
    Log(Level::Debug, source, msg);
}

void Logging::Info(const std::string& source, const std::string& msg)
{
    Log(Level::Info, source, msg);
}

void Logging::Warn(const std::string& source, const std::string& msg)
{
    Log(Level::Warn, source, msg);
}

void Logging::Error(const std::string& source, const std::string& msg)
{
    Log(Level::Error, source, msg);
}

Level Logging::GetLevel()
{
    return s_level;
}

void Logging::SetLevel(Level level)
{
    s_level = level;
}

} // namespace Aurora

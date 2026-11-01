/**
 * AuroraModuleSettings
 *
 * Header-only helper that lets an Aurora module read its own settings from the
 * AzerothCore configuration (worldserver.conf plus configs/modules/*.conf).
 *
 * Every Aurora option is namespaced "Aurora.<Module>." so that options of one
 * module can never shadow another. Values are logged under the "module.<name>"
 * log filter, which AzerothCore already routes to console and log file
 * (Logger.module in worldserver.conf) and which inherits its level from root.
 *
 * Usage:
 *   m_maxBots = Aurora::Settings::Number("Aurora.Bots.MaxBots", 20, "AuroraBots");
 */
#pragma once

#include "Config.h"
#include "Log.h"

#include <string>

namespace Aurora {
namespace Settings {

// Log filter shared by every Aurora module option message.
inline std::string Filter(char const* module)
{
    return std::string("module.") + module;
}

template<class T>
inline T Get(std::string const& option, T const& defaultValue, char const* module)
{
    T value = sConfigMgr->GetOption<T>(option, defaultValue);
    sLog->outMessage(Filter(module), LogLevel::LOG_LEVEL_INFO, "{}: {} = {}", module, option, value);
    return value;
}

// Booleans are reported as yes/no rather than 1/0 to match how they are
// written in the config files.
inline bool Enabled(std::string const& option, char const* module, bool defaultValue = true)
{
    bool value = sConfigMgr->GetOption<bool>(option, defaultValue);
    sLog->outMessage(Filter(module), LogLevel::LOG_LEVEL_INFO,
        "{}: {} = {}", module, option, value ? "yes" : "no");
    return value;
}

} // namespace Settings
} // namespace Aurora

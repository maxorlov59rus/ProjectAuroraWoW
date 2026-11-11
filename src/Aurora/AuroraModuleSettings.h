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

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

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

// Numeric option restricted to [minValue, maxValue]. Out-of-range values are
// corrected rather than rejected: a typo in a rate should not stop the server,
// but it has to be visible in the log.
template<class T>
inline T Bounded(std::string const& option, T defaultValue, T minValue, T maxValue, char const* module)
{
    T value = sConfigMgr->GetOption<T>(option, defaultValue);

    T clamped = std::clamp(value, minValue, maxValue);
    if (clamped != value)
    {
        sLog->outMessage(Filter(module), LogLevel::LOG_LEVEL_WARN,
            "{}: {} = {} is outside [{}, {}], using {}",
            module, option, value, minValue, maxValue, clamped);
    }
    else
    {
        sLog->outMessage(Filter(module), LogLevel::LOG_LEVEL_INFO, "{}: {} = {}", module, option, value);
    }

    return clamped;
}

// Comma separated list, e.g. "WARRIOR,PALADIN". Empty entries are dropped, so
// an option written as "" yields an empty list rather than one empty item.
inline std::vector<std::string> List(std::string const& option, std::string const& defaultValue, char const* module)
{
    auto trim = [](std::string& text)
    {
        auto const notSpace = [](unsigned char c) { return !std::isspace(c); };
        text.erase(text.begin(), std::find_if(text.begin(), text.end(), notSpace));
        text.erase(std::find_if(text.rbegin(), text.rend(), notSpace).base(), text.end());
    };

    std::string const raw = sConfigMgr->GetOption<std::string>(option, defaultValue);

    std::vector<std::string> values;
    std::istringstream stream(raw);
    std::string item;
    while (std::getline(stream, item, ','))
    {
        trim(item);
        if (!item.empty())
            values.push_back(std::move(item));
    }

    sLog->outMessage(Filter(module), LogLevel::LOG_LEVEL_INFO, "{}: {} = {}", module, option, raw);
    return values;
}

// Case-insensitive membership test for the List() options above.
inline bool Contains(std::vector<std::string> const& values, std::string const& value)
{
    auto equal = [&value](std::string const& item)
    {
        return item.size() == value.size()
            && std::equal(item.begin(), item.end(), value.begin(), [](char l, char r)
            {
                return std::toupper(static_cast<unsigned char>(l)) == std::toupper(static_cast<unsigned char>(r));
            });
    };

    return std::find(values.begin(), values.end(), value) != values.end()
        || std::any_of(values.begin(), values.end(), equal);
}

} // namespace Settings
} // namespace Aurora

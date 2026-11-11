/**
 * AuroraSkins
 *
 * Entry point for Aurora-specific character skins. The module can be used
 * to:
 *  - load a removable skin table (e.g. `character_skin` in DB);
 *  - reduce the visual representation of a character (outline, wireframe);
 *  - disable/enable client-side "skin" rendering features.
 *
 * Reminder:
 *  - Client-side skin switching is done by the game client. The server
 *    only sends skin flags/status. The exact client-side UI belongs to
 *    your custom WoW client, not to this module.
 *  - Do not distribute protected WoW client files. Use your own client build.
 */
#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Aurora {

class Skins
{
public:
    Skins();
    ~Skins();

    // Called once when the module is loaded
    bool Initialize();

    // Called when the module is unloaded
    void Shutdown();

    // Returns the default skin identifier for a character.
    // Hook your server's character->skin update logic here.
    static std::string GetSkinFor(uint32_t characterId);

    // Sets the identifier GetSkinFor() reports for characters without an
    // explicit skin. AuroraSkins calls it with Aurora.Skins.Default at startup.
    static void SetDefaultSkin(const std::string& skinId);
    static std::string const& GetDefaultSkin();

    // Registers a skin change listener. Called by AuroraTools on client
    // skin sync events.
    static void RegisterSkinListener(std::function<void(uint32_t, const std::string&)> listener);

private:
    static std::vector<std::function<void(uint32_t, const std::string&)>> s_listeners;
    static std::string s_defaultSkin;
};

} // namespace Aurora

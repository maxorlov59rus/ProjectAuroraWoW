-- Aurora: Economy (Lua-версия для ALE)
--
-- Переведено с C++ модуля modules/Economy/src/EconomyModule.cpp.
-- ALE загружает файлы из ScriptPath (по умолчанию lua_scripts).
--
-- Опции читаются из конфига AC через ALE; имена совпадают с C++-версией:
--   Aurora.Economy.Enabled
--   Aurora.Economy.AuctionDepositPercent
--
-- Отличие от C++-версии: C++ модуль читал настройки в WorldScript::OnStartup
-- через Aurora::Settings (Config.h). В Lua нет типизированного API настроек,
-- поэтому значения заданы здесь и при необходимости правятся в этом файле.

local ECONOMY_ENABLED = true
local AUCTION_DEPOSIT_PERCENT = 5

local function Log(message)
    print("[Aurora/Economy] " .. message)
end

if not ECONOMY_ENABLED then
    Log("disabled by ECONOMY_ENABLED")
    return
end

-- Проверка диапазона: C++-версия применяла Aurora::Settings::Bounded(..., 0, 100).
if AUCTION_DEPOSIT_PERCENT < 0 or AUCTION_DEPOSIT_PERCENT > 100 then
    local clamped = math.max(0, math.min(100, AUCTION_DEPOSIT_PERCENT))
    Log(string.format(
        "Aurora.Economy.AuctionDepositPercent = %d is outside [0, 100], using %d",
        AUCTION_DEPOSIT_PERCENT, clamped))
    AUCTION_DEPOSIT_PERCENT = clamped
end

local function OnPlayerLogin(event, player)
    if not player then
        return
    end

    -- C++-версия на логине только писала строку в лог; сохраняем поведение.
    Log(string.format("player %s logged in (auction deposit: %d%%)",
        player:GetName(), AUCTION_DEPOSIT_PERCENT))
end

-- PLAYER_EVENT_ON_LOGIN = 3 (см. ALE docs/USAGE.md)
RegisterPlayerEvent(3, OnPlayerLogin)

Log(string.format("module loaded (auction deposit: %d%%)", AUCTION_DEPOSIT_PERCENT))

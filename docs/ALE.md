# ALE (AzerothCore Lua Engine) в проекте Aurora

## Что это

`mod-ale` — независимый Lua-движок для AzerothCore (форк Eluna, но
несовместимый с оригинальными Eluna-скриптами). Лицензия **GPLv3** —
совместима с Aurora.

Локальная копия лежит в `C:\Users\Orel\Downloads\mod-ale-master` и уже
установлена в деревья AC:
- `C:\Users\Orel\Downloads\azerothcore-wotlk\modules\mod-ale`
- `E:\azeroth\modules\mod-ale`

## Как ALE загружает скрипты

`conf/mod_ale.conf.dist`:

```ini
ALE.Enabled = true
ALE.ScriptPath = "lua_scripts"
```

Скрипты кладутся в `lua_scripts/` рядом с `worldserver.exe` (или по
указанному пути). Все `.lua` подхватываются рекурсивно.

## Наши скрипты

```
lua_scripts/
└── aurora/
    └── economy.lua
```

Правило: один модуль Aurora — один файл в `lua_scripts/aurora/`.

## Перевод модуля с C++ на Lua

Когда модуль стоит переводить:

| Признак | C++ | Lua через ALE |
|---|---|---|
| Только команды чата и хуки игрока | Нужна пересборка ядра | Правка файла на лету |
| Чтение настроек из `Aurora.<Module>.*` | Через `Aurora::Settings` | Константы в файле |
| Сложная логика с БД и состоянием | C++ предпочтительнее | Работа возможна, но медленнее |
| Модуль должен работать без ALE | C++ | Нельзя |

## Текущий статус перевода

| Модуль | C++ | Lua | Примечание |
|---|---|---|---|
| Economy | `modules/Economy` | `lua_scripts/aurora/economy.lua` | **Переведён**. C++-версия оставлена, чтобы не ломать сборку |

## Ограничения Lua-версии

- **Настройки не читаются из `worldserver.conf`.** В C++-версии были
  `Aurora.Economy.Enabled` и `Aurora.Economy.AuctionDepositPercent` через
  `Aurora::Settings::Bounded` с диапазоном 0..100. В Lua значения заданы
  константами в начале файла, с той же проверкой диапазона.
- **`PLAYER_EVENT_ON_LOGIN = 3`** — числовой ID из `docs/USAGE.md` ALE.
  Если ALE сменит нумерацию, хук перестанет срабатывать молча.
- **Логирование** идёт через `print()` в консоль сервера, а не в
  `Log.h`-канал AC. Формат строки совпадает: `[Aurora/Economy] ...`.

## Что не проверено

Lua-скрипт **не запускался** — для этого нужен работающий `worldserver`
с ALE. Проверена только корректность API по документации ALE
(`RegisterPlayerEvent`, `player:GetName()`).

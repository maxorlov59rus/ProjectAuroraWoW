# Architecture — Aurora на AzerothCore

## Структура системы

```
┌──────────────────────────────────────────────────────────────┐
│  World of Warcraft Wotlk-клиент  (WoW Client)                 │
│  ── TCP к WorldServer (порт 8085 по умолчанию) ──            │
│  ── Вход в AuthServer (порт 3724) ──                         │
└───────────────────────────┬──────────────────────────────────┘
                            │ WoW-пакеты
┌───────────────────────────▼──────────────────────────────────┐
│  AzerothCore (World of Warcraft Wotlk server emulator)       │
│  WorldServer  (map, NPC, combat, quests, items, loot...)     │
│  AuthServer   (логины, realm list)                           │
│  Modules      (static-линковка, см. docs/BUILDING.md)        │
└───────────────────────────┬──────────────────────────────────┘
                            │ расширения/кастомизация
┌───────────────────────────▼──────────────────────────────────┐
│  Aurora (ваш проект, C++17)                                   │
│  src/Aurora/           — кастомная бизнес-логика (AuroraCore) │
│  modules/              — модули, собираемые в worldserver     │
│  src/AuroraTools/      — утилиты БД и чтения INI              │
└──────────────────────────────────────────────────────────────┘
```

## Процесс подключения клиента

```
Клиент WoW → AuthServer (TCP 3724) → проверка логина/пароля
AuthServer → выдаёт список реалмов и адрес WorldServer
Клиент → WorldServer (TCP 8085, конфигурируется в worldserver.conf)
WorldServer → инициализация мира, квесты, NPC, карты, PvP
```

## Как Aurora кастомизирует AzerothCore

1. **`src/Aurora/`** — слой `AuroraCore` (static lib). Собирается **внутрь**
   дерева AC в `src/server/AuroraCore/` и линкуется в `game-interface`,
   поэтому его заголовки видны всем модулям. Содержит:
   - `AuroraLogging` — обёртка над диагностикой;
   - `AuroraModuleSettings` — типизированное чтение `Aurora.<Module>.*`;
   - `AuroraSkills`, `AuroraSkins`, `AuroraGameMode`, `AuroraInterface`;
   - `AuroraBot` — базовый класс бота (требует AC-заголовков, сейчас не в target).
2. **`modules/Aurora*`, `modules/<Name>`** — модули AzerothCore,
   **статически** собираемые в `worldserver.exe` (`MODULE_<NAME> = static`).
   Точка входа — функция `Add<Name>Scripts()`, которую вызывает
   сгенерированный `ModulesLoader.cpp` из `sScriptMgr->Initialize()`.
3. **`conf/`** — корневой каталог проекта не содержит конфигов;
   рабочие файлы создаются рядом с бинарниками AC
   (`configs/worldserver.conf`, `configs/modules/aurora_*.conf`).
4. **`databases/`** — дополнительные таблицы Aurora
   (`campaign_*`, `bots`, `classless_players`). Основные схемы
   (`acore_auth`, `acore_characters`, `acore_world`) создаёт установщик AC.

## Зависимости

- C++17
- CMake ≥ 3.16
- Windows: Visual Studio 2022, MSVC
- Linux: GCC ≥ 11 / Clang ≥ 14, Ninja или Make
- MySQL 5.7 или 8.x (AzerothCore требует MySQL, не PostgreSQL)
- Boost ≥ 1.78 (Windows — статические `.lib` для toolset)
- Python 3.9+ — только для скриптов миграции, в рантайме не нужен

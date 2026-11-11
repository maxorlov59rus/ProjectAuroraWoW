# Architecture — Aurora на AzerothCore

## Структура системы

```
┌──────────────────────────────────────────────────────────────┐
│  World of Warcraft Wotlk-клиент  (WoW Client)                 │
│  ── TCP/UDP с WorldServer (1119-1121, пакеты WOTLK) ──       │
│  ── Вход в AuthServer для логина ──                          │
└───────────────────────────┬──────────────────────────────────┘
                            │ WoW-пакеты
┌───────────────────────────▼──────────────────────────────────┐
│  AzerothCore (World of Warcraft Wotlk server emulator)       │
│  WorldServer  (map, NPC, combat, quests, items, loot...)     │
│  AuthServer   (рассылка ключей, логины, группы)              │
│  Modules      (триал, квесты, бои и т.д.)                    │
└───────────────────────────┬──────────────────────────────────┘
                            │ расширения/кастомизация
┌───────────────────────────▼──────────────────────────────────┐
│  Aurora (ваш проект, C++17)                                   │
│  src/Aurora/           — кастомная бизнес-логика              │
│  modules/              — загружаемые модули                    │
│  src/AuroraTools/      — загрузка и администрирование БД       │
└────────────────────────────────────────────────────────────────┘
```

## Процесс запроса клиентом

```
Клиент WoW → запрос в AuthServer (TCP 3724/8085)
AuthServer → выдаёт ключ сессии / рассылает сессию
Клиент → выбирает сервер → подключение к WorldServer (1119/1121 TCP)
WorldServer → инициализация мира, квесты, комбо, npcs, мапы, pvp
```

## Как Aurora кастомизирует AzerothCore

1. **src/Aurora/** — «ядро» кастомизации. Не является загружаемым модулем,
   а собирается внутрь проекта и предоставляет хуки/минфикторы/отдельный логи.
2. **modules/AuroraSkills**, **AuroraSkins**, **AuroraGameModes** —
   загружаемые AzerothCore модули (`.dll`/`.so`), подключаются через
   `modules/` конфиг сервера и получают полный доступ к API AzerothCore.
3. **conf/** — конфигурации серверов (authserver.conf, worldserver.conf),
   переопределённые для Aurora.
4. **databases/** — схема БД, подходящая под AzerothCore + Aurora-таблицы.

## Зависимости

- C++17
- CMake ≥ 3.16
- Windows: Visual Studio 2022, MSVC
- Linux: GCC ≥ 11 / Clang, Ninja или Make
- MySQL 5.7 или 8.x (AzerothCore требует MySQL, не PostgreSQL)
- Babelfish (для преобразования C++ → .wow пакеты при необходимости)

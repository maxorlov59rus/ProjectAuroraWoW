# Aurora

**Aurora** — кастомный проект MMORPG на базе **World of Warcraft Wotlk-клиента** и серверного ядра **AzerothCore**.

## Что это

| Часть | Назначение |
|---|---|
| **Клиент** | World of Warcraft Wotlk (`.exe`/`.dll`/`.pak`). Играет на клиенте WoW. |
| **Сервер** | AzerothCore (WorldServer + AuthServer, C++17, CMake). |
| **Фича-сайт** | Проект `Aurora` оформляет слой кастомизации: режимы игры, навыки, скины, UI/интерфейс. |

Aurora — это **не самостоятельный сервер**, а слой навешиваний и конфигурация над AzerothCore. Всё, что касается пакетов, БД, сетевой логики,_npc_, классов и т.д., берётся из AzerothCore.

## Структура

```
Aurora/
├── CMakeLists.txt              # главный билд (подключает AzerothCore)
├── CMakePresets.json           # пресеты для VS / CLion / VS Code
├── README.md
├── LICENSE                     # GPLv3 + кастомная лицензия
├── scripts/
│   ├── build_win.bat           # сборка под Windows
│   ├── build_linux.sh          # сборка под Linux
│   ├── run_world.sh            # запуск WorldServer
│   └── run_auth.sh             # запуск AuthServer
├── conf/                       # ваши конфиги (серверы, world, auth)
├── databases/                  # SQL-схема Чары (Auth + World + Characters)
├── docs/                       # эта документация + скрипты подготовки БД
├── src/
│   ├── Aurora/                 # ваш кастомный слой (логика, интерфейс, логи)
│   ├── AuroraServer/           # обёртки запуска серверов
│   └── AuroraTools/            # утилиты (Загрузка БД, чтение INI)
├── modules/
│   ├── AuroraGameModes/        # режимы игры
│   ├── AuroraSkills/           # навыки
│   └── AuroraSkins/            # скины персонажей
├── externals/
│   └── WoWClient/              # ЗДЕСЬ ВЫ КЛАДЁТЕ WoW Wotlk-клиент
└── docker/                     # контейнеры для Linux
```

## Быстрый старт (Windows)

### 1. Подготовка

1. Скачайте/положите **AzerothCore Wotlk** по пути `E:/azerothcore-wotlk-master` (или укажите другой при конфигурации).
2. Скопируйте **WoW Wotlk-клиент** в `Aurora/externals/WoWClient/`.
3. Установите **Microsoft Visual Studio 2022** с компонентами:
   - «C++ наборы разработчика»
   - «Windows 10/11 SDK»
   - «Средства сборки CMake»
4. Установите **MySQL** или **MariaDB** (5.7 / 8.x) и запустите сервер.

### 2. Конфигурация БД

Импортируйте схему из `Aurora/databases/`, либо from `E:/azerothcore-wotlk-master/:` (при необходимости скопируйте схему из оригиналов AzerothCore в `Aurora/databases/`).

```
mysql -u root -p < Aurora/databases/Auth.sql
mysql -u root -p < Aurora/databases/World.sql
```

### 3. Сборка

**Способ A — Пресеты CLion / VS Code:**

```
cmake --preset windows-ninja
cmake --build --preset windows-ninja
```

**Способ B — Visual Studio 2022:**

```
cmake -S . -B build/vs2022 -G "Visual Studio 17 2022"
cmake --build build/vs2022 --config Release
```

**Способ C — Скрипт:**

```
scripts\build_win.bat
```

### 4. Запуск

```
scripts\run_auth.sh     # Windows: scripts\run_auth.bat
scripts\run_world.sh    # Windows: scripts\run_world.bat
```

Сервер запустится с конфигами из `Aurora/conf/`.

## Лицензии

- **AzerothCore** — `LICENSE` в `E:/azerothcore-wotlk-master` (GPLv2 / LGPLv3).
- **Aurora** — собственная лицензия `LICENSE` в корне проекта (GPLv3).
- **World of Warcraft Wotlk-клиент** — защищён законодательством Blizzard Entertainment. Проект Aurora использует клиент только в собственной локальной среде пользователя и не копирует/не распространяет защищённый контент.

См. `docs/LICENSE_INFO.md` для подробностей.

## Лицензия Aurora

См. `LICENSE` в корне проекта.

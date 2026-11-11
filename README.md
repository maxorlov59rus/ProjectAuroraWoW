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

> Сборка выполняется **в дереве AzerothCore**, а не в этой папке. Подробности:
> [`docs/BUILDING.md`](docs/BUILDING.md).

### 1. Подготовка

1. Клонируйте **AzerothCore Wotlk** куда угодно, например в `E:\azerothcore-wotlk-master`.
2. Установите **Microsoft Visual Studio 2022** с компонентами:
   - «C++ наборы разработчика»
   - «Windows 10/11 SDK»
   - «Средства сборки CMake»
3. Установите **MySQL** или **MariaDB** (5.7 / 8.x) и запустите сервер.
4. Соберите **Boost** статически (см. `docs/BUILDING.md`).

### 2. БД

Базовые схемы создаёт установщик AzerothCore
(`data/sql/create/create_mysql.sql`). Дополнительные таблицы Aurora:

```
mysql -u root -p acore_world < databases/CampaignOfLight.sql
mysql -u root -p acore_world < databases/CampaignOfLight_data.sql
```

### 3. Миграция и сборка

```
python scripts\migrate_to_ac.py --ac E:\azerothcore-wotlk-master
cmake -S E:\azerothcore-wotlk-master -B E:\azerothcore-wotlk-master\build ^
      -G "Visual Studio 17 2022" -DBoost_ROOT=C:\local\boost_1_92_0\vc143
cmake --build E:\azerothcore-wotlk-master\build --config RelWithDebInfo
```

`--dry-run` у скрипта миграции покажет, что будет изменено, без записи файлов.

### 4. Запуск

```
cd E:\azerothcore-wotlk-master\build\bin\RelWithDebInfo
authserver.exe
worldserver.exe
```

Конфиги лежат рядом с бинарниками: `configs/worldserver.conf`,
`configs/authserver.conf`, `configs/modules/aurora_*.conf`.

## Лицензии

- **AzerothCore** — `LICENSE` в `E:/azerothcore-wotlk-master` (GPLv2 / LGPLv3).
- **Aurora** — собственная лицензия `LICENSE` в корне проекта (GPLv3).
- **World of Warcraft Wotlk-клиент** — защищён законодательством Blizzard Entertainment. Проект Aurora использует клиент только в собственной локальной среде пользователя и не копирует/не распространяет защищённый контент.

См. `docs/LICENSE_INFO.md` для подробностей.

## Лицензия Aurora

См. `LICENSE` в корне проекта.

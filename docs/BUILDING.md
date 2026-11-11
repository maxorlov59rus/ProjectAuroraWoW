# Сборка Aurora на AzerothCore

## Архитектура сборки

Aurora — это **не самостоятельный CMake-проект**. Единственная поддерживаемая
схема: AzerothCore является верхним CMake-проектом, а исходники Aurora
копируются в его дерево скриптом миграции.

```
azerothcore-wotlk/                       <- верхний CMake-проект
├── src/server/AuroraCore/               <- слой Aurora (static lib), шаг 2
├── modules/<Name>/src/*.cpp             <- модули Aurora (static-линковка в worldserver)
└── conf/config.cmake                    <- MODULE_<NAME> = static | disabled
```

Корневой `CMakeLists.txt` в Aurora-репозитории оставлен только для IDE
(навигация по файлам и подсветка). Он **не является** способом сборки
рабочего сервера: `add_subdirectory(azerothcore)` ломает конфигурацию AC,
потому что AC резолвит свои макросы, `conf/` и `modules/` относительно
`CMAKE_SOURCE_DIR`.

## Требования

| Компонент | Windows | Linux |
|---|---|---|
| Сборщик | Visual Studio 2022 (MSVC) | GCC ≥ 11, Clang ≥ 14, Ninja |
| CMake | ≥ 3.16 | ≥ 3.16 |
| Boost | ≥ 1.78, **статические** lib для вашего toolset | ≥ 1.74 |
| БД | MySQL 5.7/8 | MySQL 5.7/8 |
| Python | 3.9+ (для скриптов миграции) | 3.9+ |

### Boost (Windows)

AC требует собранные библиотеки Boost, а не только заголовки. Если готовые
`.lib` вашего toolset нет, соберите нужные компоненты:

```bat
cd C:\local\boost_1_92_0
bootstrap.bat
b2 toolset=msvc-14.3 --with-filesystem --with-program_options ^
   --with-iostreams --with-regex --with-thread ^
   --stagedir=vc143 stage variant=debug,release ^
   link=static runtime-link=shared threading=multi address-model=64
```

## 1. Миграция исходников Aurora в дерево AC

```bat
python scripts\migrate_to_ac.py --ac E:\azerothcore-wotlk-master
```

Полезные флаги:

```
--ac PATH        путь к дереву AC (обязательный)
--aurora PATH    путь к репозиторию Aurora (по умолчанию — родитель скрипта)
--dry-run        только показать, что будет сделано, ничего не записывая
```

Скрипт делает шесть шагов:

1. Прописывает `add_subdirectory(AuroraCore)` в `src/server/CMakeLists.txt`
   AC и добавляет `AuroraCore` в `game-interface`, чтобы include-пути
   core-слоя были видны всем модулям.
2. Копирует `Aurora/src/Aurora/*` в `src/server/AuroraCore/` AC.
3. Копирует `Aurora/modules/*/src/*` и `Aurora/modules/*/conf/*.conf.dist` в
   `AC/modules/*/`, дописывая каждому `.cpp` точку входа `Add<Name>Scripts()`,
   которую вызывает генерируемый AC `ModulesLoader.cpp`.
4. Пишет `AC/conf/config.cmake` со списком включённых/отключённых модулей.
5. Создаёт рабочие конфиги рядом с бинарниками: `configs/worldserver.conf`,
   `configs/authserver.conf` и `configs/modules/aurora_*.conf`.
   Существующие файлы не перезаписываются.
6. Кладёт рядом с `worldserver.exe` runtime-библиотеки: `libmysql.dll`,
   `libcrypto-*.dll`, `libssl-*.dll` и `legacy.dll`. Без `legacy.dll` AC
   падает assert'ом ещё до вывода лога.

Файлы AC остаются вне git-репозитория Aurora — единственным источником
истины для кода модулей считается Aurora-репозиторий.

### Управление списком модулей

Порт модуля считается завершённым, когда он компилируется. Включённые модули
перечислены в `ENABLED` в начале `scripts/migrate_to_ac.py`; остальные
прописываются в `conf/config.cmake` как `disabled`, чтобы не ломать сборку.

## 2. Конфигурация и сборка

Работаем **в дереве AzerothCore**, не в Aurora:

```bat
cmake -S E:\azerothcore-wotlk-master -B E:\azerothcore-wotlk-master\build ^
      -G "Visual Studio 17 2022" -DBoost_ROOT=C:\local\boost_1_92_0\vc143
cmake --build E:\azerothcore-wotlk-master\build --config RelWithDebInfo
```

Быстрая проверка одного модуля без сборки всего ядра — цель `modules`:

```bat
cmake --build E:\azerothcore-wotlk-master\build --target modules --config RelWithDebInfo
```

## 3. БД

Схема characters/world/auth создаётся установщиком AC:

```bash
mysql -u root -p < E:/azerothcore-wotlk-master/data/sql/create/create_mysql.sql
```

Дополнительные таблицы Aurora лежат в `Aurora/databases/`:

```bash
mysql -u root -p acore_world < Aurora/databases/CampaignOfLight.sql
mysql -u root -p acore_world < Aurora/databases/CampaignOfLight_data.sql
```

> Внимание: `CampaignOfLight.sql` и `CampaignOfDarkness.sql` создают
> одинаковую таблицу `campaign_npcs`. Импортируйте **один** из них, если
> планируете использовать обе кампании — см. TODO в issue-трекере.

## 4. Запуск

```bat
cd E:\azerothcore-wotlk-master\build\bin\RelWithDebInfo
authserver.exe
worldserver.exe
```

Рабочие конфиги лежат рядом: `configs/worldserver.conf`,
`configs/authserver.conf`, `configs/modules/aurora_*.conf`.

## Конфигурация модулей

Каждый модуль описывает настройки в `modules/<Name>/conf/aurora_*.conf.dist`
по формату AC: `Key = value`, `#` — комментарий. Опции называются
`Aurora.<Module>.*`, чтобы модули не перекрывали настройки друг друга.

Рабочие файлы читаются из `configs/modules/<имя>.conf`; их список
(`CONFIG_FILE_LIST`) формируется glob'ом по `conf/*.conf.dist` **во время
CMake-конфигурации**, поэтому после добавления нового `.conf.dist` нужно:

```bat
python scripts\migrate_to_ac.py --ac E:\azerothcore-wotlk-master
cmake -S E:\azerothcore-wotlk-master -B E:\azerothcore-wotlk-master\build
```

Модули читают свои опции в `WorldScript::OnStartup` — этот хук выполняется уже
после `sConfigMgr->LoadModulesConfigs()`. Проверить, что конфиги подхватились,
можно без БД:

```bat
worldserver.exe --dry-run
```

в выводе должен быть блок `Loading Modules Configuration...` со списком
`aurora_*.conf`.

## Частые проблемы

**`Could NOT find Boost`** — нет статических `.lib` для вашего toolset.
Соберите Boost (см. выше) и передайте `-DBoost_ROOT=<путь>`.

**Модуль не виден в графе сборки** — проверьте `MODULE_<NAME>` в
`AC/conf/config.cmake` (`static` / `disabled`) и перезапустите скрипт
миграции после правок в `ENABLED`.

**`error C2065: uint32: необъявленный идентификатор`** — в заголовке модуля
не хватает `#include "Define.h"` (типы `uint8`/`uint32` определяет AC, а не
`<cstdint>`).

**`override` не переопределяет метод базового класса** — сигнатура хука
разошлась с текущей версией AC. Актуальные сигнатуры смотрите в
`src/server/game/Scripting/ScriptDefines/<Kind>Script.h`.

**ACCESS_VIOLATION в `ScriptRegistry<T>::_checkMemory` при запуске** — скрипт
создан файловым объектом (`namespace { FooScript sFoo; }`). Такую регистрацию
надо делать внутри `Add<Name>Scripts()`. Перевод существующих модулей:
`python scripts\convert_script_registration.py`.

**`Not found 'legacy.dll'`** — рядом с `worldserver.exe` нет провайдера
`legacy.dll` и библиотек OpenSSL/MySQL. Их раскладывает шаг 6
`migrate_to_ac.py`.

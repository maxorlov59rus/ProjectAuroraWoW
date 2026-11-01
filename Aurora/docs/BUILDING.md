# Инструкция по сборке

## Архитектура сборки

Aurora использует **штатный механизм AzerothCore**: AC является верхним
CMake-проектом, а модули Aurora подключаются через его собственный механизм
`modules/`. Это единственная конфигурация, которую поддерживает upstream.

```
azerothcore-wotlk (верхний CMake-проект)
├── src/server/AuroraCore/   <- Aurora core layer (static lib)
├── modules/<Name>/src/      <- модули Aurora (static-линковка в worldserver)
└── conf/config.cmake        <- выбор включённых модулей
```

Почему так, а не `add_subdirectory(azerothcore)`: AC резолвит свои макросы,
`conf/` и `modules/` относительно `CMAKE_SOURCE_DIR`. При вложенном
подключении `CMAKE_SOURCE_DIR` указывает на Aurora, и конфигурация AC падает.

## Требования

| Компонент | Windows | Linux |
|---|---|---|
| Сборщик | Visual Studio 2022 (MSVC) | GCC ≥ 11, Clang ≥ 14, Ninja |
| CMake | ≥ 3.16 | ≥ 3.16 |
| Boost | ≥ 1.78, **статические** lib для вашего toolset | ≥ 1.74 |
| БД | MySQL 5.7/8 | MySQL 5.7/8 |

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

Исходники модулей живут в репозитории Aurora (`Aurora/modules/`), а в дерево
AC копируются скриптом миграции. Он идемпотентен: правки в Aurora-репозитории
достаточно перенести повторным запуском.

```bat
python scripts\migrate_to_ac.py
```

Скрипт делает четыре вещи:

1. Прописывает `add_subdirectory(AuroraCore)` в `src/server/CMakeLists.txt`
   AC и добавляет `AuroraCore` в `game-interface`, чтобы include-пути
   core-слоя были видны всем модулям.
2. Копирует `Aurora/src/Aurora/*` в `src/server/AuroraCore/` AC.
3. Копирует `Aurora/modules/*/src/*` в `AC/modules/*/src/`, дописывая каждому
   `.cpp` точку входа `Add<Name>Scripts()`, которую вызывает генерируемый
   AC `ModulesLoader.cpp`.
4. Пишет `AC/conf/config.cmake` со списком включённых/отключённых модулей.

Файлы AC остаются вне git-репозитория Aurora — единственным источником истины
для кода модулей считается Aurora-репозиторий.

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

Быстрая проверка одного модуля без сборки всего ядра — цель `modules`
(собирает статическую библиотеку со всеми включёнными модулями):

```bat
cmake --build E:\azerothcore-wotlk-master\build --target modules --config RelWithDebInfo
```

## 3. БД

```bash
mysql -u root -p < Aurora/databases/Auth.sql
mysql -u root -p < Aurora/databases/World.sql
mysql -u root -p < Aurora/databases/Characters.sql
```

## 4. Запуск

### Windows
```bat
scripts\run_auth.bat
scripts\run_world.bat
```

### Linux
```bash
./scripts/run_auth.sh RelWithDebInfo
./scripts/run_world.sh RelWithDebInfo
```

## Структура бинарников после сборки

```
azerothcore-wotlk/build/bin/RelWithDebInfo/
├── authserver.exe
├── worldserver.exe
└── skills/                 (dynamic-модули, если MODULES=dynamic)
```

При `MODULES=static` (значение по умолчанию в `conf/config.cmake`) код
модулей уже встроен в `worldserver.exe`, отдельных `.dll` не требуется.

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

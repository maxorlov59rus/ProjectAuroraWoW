# Инструкция по сборке

## Требования

| Компонент | Windows | Linux |
|---|---|---|
| Сборщик | Visual Studio 2022 (MSVC) | GCC ≥ 11, Clang ≥ 14, Ninja |
| CMake | ≥ 3.16 | ≥ 3.16 |
| БД | MySQL 5.7/8 | MySQL 5.7/8 |
| Библиотеки | MSVC RTL, WinSock, WS2_32 | glibc, libmysqlclient |

## 1. Подготовка среды

### Windows
1. Установите **Visual Studio 2022** (Community/Professional/Enterprise). При установке отметьте:
   - `Desktop development with C++`
   - `Windows 10/11 SDK`
   - `CMAKE`
2. Установите **MySQL** (инструмент `mysql.exe` в `PATH`).
3. Скопируйте WoW-клиент в `Aurora/externals/WoWClient/`.

### Linux
```bash
sudo apt-get install -y build-essential cmake ninja-build mysql-server libmysqlclient-dev
```

## 2. Конфигурация пути к AzerothCore

В `Aurora/CMakeLists.txt` по умолчанию указан путь:

```cmake
set(AZEROTHCORE_SOURCE_DIR "E:/azerothcore-wotlk-master" CACHE PATH ...)
```

Если AzerothCore лежит в другом месте, пересобирите с:

```bash
cmake -S . -B build -DAZEROTHCORE_SOURCE_DIR=/mnt/azerothcore-wotlk-master
```

## 3. Сборка

### Вариант 1 — пресеты CMake (рекомендуется)
```bash
cmake --preset windows-ninja     # или linux-ninja
cmake --build --preset windows-ninja
```

### Вариант 2 — вручную
```bash
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config RelWithDebInfo
```

### Вариант 3 — скрипт
```bat
scripts\build_win.bat
```

```bash
./scripts/build_linux.sh
```

## 4. БД

Импортируйте схему Чары:

```bash
mysql -u root -p < Aurora/databases/Auth.sql
mysql -u root -p < Aurora/databases/World.sql
mysql -u root -p < Aurora/databases/Characters.sql
```

Настройте соединение с БД в `conf/authserver.conf` и `conf/worldserver.conf`
переменными `mysql hostname`, `mysql user`, `mysql pass`.

## 5. Запуск

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
build/
├── bin/
│   ├── authserver.exe/.so
│   └── worldserver.exe/.so
├── lib/
└── src/Aurora/...
```

Авто-модули Aurora (`AuroraSkills`, `AuroraSkins`, `AuroraGameModes`) должны
находиться в каталоге `conf/modules/` (или в каталоге модулей, указанном в
`worldserver.conf`), иначе модуль-загрузчик не их найдёт.

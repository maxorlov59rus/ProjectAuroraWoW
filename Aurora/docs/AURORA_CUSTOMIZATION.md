# Как добавить кастомизацию «Аврора»

## 1. Скины персонажей (AuroraSkins)

Модуль `AuroraSkills` — это загружаемый AzerothCore-модуль. Структура:

```
modules/AuroraSkins/
├── CMakeLists.txt
├── AuroraSkinsModule.cpp
└── AuroraSkinsModule.h
```

Пример `CMakeLists.txt` (стандарт для AzerothCore):

```cmake
add_library(AuroraSkins MODULE
    AuroraSkinsModule.cpp
    AuroraSkinsModule.h
)

target_link_libraries(AuroraSkins PRIVATE
    AzerothCoreLib
    ${MYSQL_LIBRARIES}
)

set_target_properties(AuroraSkins PROPERTIES
    PREFIX "mod_"
    OUTPUT_NAME "AuroraSkins"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
```

Зарегистрируйте модуль в `worldserver.conf`:

```ini
modules
{
    AuroraSkins.dll
}
```

## 2. Кастомизация игровых режимов (AuroraGameModes)

Размещайте в `modules/AuroraGameModes/` схожим образом. Там вы можете:

- переопределять функции инициализации сцены;
- подключать свои боевые режимы;
- управлять петлей режимов (мгновенный запуск, тренировки и т.д.).

## 3. Навыки (AuroraSkills)

В модуле `AuroraSkills` вы можете:

- добавлять кастомерские навыки;
- перехватывать расчёт очков опыта;
- дополнять таблицу навыков (DB); — обратитесь к исходникам AzerothCore
  для построения `character_db2.sql` с вашими навыками.

## 4. Интерфейс и логотип (AuroraInterface)

В `docs/AURORA_CUSTOMIZATION.md` описано, как:

- менять текстовые строки сервера;
- подменять логотип и скины клиентского UI;
- подключать свои .pak-файлы/ресурсы, если клиент разрешает.

> Важно: WoW-клиент не разрешает гибкую замену UI через конфиг сервера.
> Любые изменения в клиенте (скрины, иконки, локализация) делаются локально
> и недоступны при обновлении клиента. Для массового распространения
> изменений нужен модифицированный клиент, который мы не распространяем.

## 5. Настроки

Все кастомизации читаются из `Aurora/conf/` через AzerothCore-конфиг:

```ini
aurora
{
    enabled yes
    log_level info
    game_mode standard
    skin_default "default"
}
```

Параметры читаются в `src/Aurora/` через структуру настроек AzerothCore
(`Config*` API) или через свой кастомный парсер (см. `AuroraTools`).

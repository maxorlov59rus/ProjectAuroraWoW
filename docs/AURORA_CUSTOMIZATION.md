# Как добавить кастомизацию «Аврора»

Все модули Aurora собираются **статически** в `worldserver.exe` через
механизм модулей AzerothCore (`MODULE_<NAME> = static` в
`AC/conf/config.cmake`). Отдельные `.dll` не создаются. Подробнее о сборке —
[`BUILDING.md`](BUILDING.md).

## Общая структура модуля

```
modules/<Name>/
├── conf/aurora_<name>.conf.dist   # опции Aurora.<Module>.*
└── src/
    ├── <Name>Module.cpp           # точка входа Add<Name>Scripts()
    └── <Name>Module.h             # опционально
```

В `.cpp` обязательна функция:

```cpp
void Add<Name>Scripts()
{
    new <Name>WorldScript();   // скрипты создаются здесь, не статиками
}
```

Её вызывает сгенерированный `ModulesLoader.cpp` из `sScriptMgr->Initialize()`.
Скрипт внутри читает опции в `OnStartup()`:

```cpp
void OnStartup() override
{
    if (!Aurora::Settings::Enabled("Aurora.<Module>.Enabled", "<Name>"))
        return;

    m_value = Aurora::Settings::Bounded<int>(
        "Aurora.<Module>.Value", 10, 0, 100, "<Name>");
}
```

Нельзя создавать скрипты файловыми объектами
(`namespace { FooScript sFoo; }`) — это падает в
`ScriptRegistry<T>::_checkMemory`. Конвертер старых модулей:
`python scripts/convert_script_registration.py`.

## 1. Скины персонажей (AuroraSkins)

Модуль `AuroraSkins` читает `Aurora.Skins.Enabled` и
`Aurora.Skins.Default`. Значение по умолчанию попадает в
`Aurora::Skins::SetDefaultSkin()` и отдаётся `GetSkinFor()` для персонажей
без явного скина.

Клиентская часть не управляется сервером: сервер передаёт только идентификатор
скина, отрисовка — на стороне клиента.

## 2. Игровые режимы (AuroraGameModes)

Настраиваются `Aurora.GameModes.Allowed` (список допустимых) и
`Aurora.GameModes.Default`. Если `Default` отсутствует в `Allowed`, берётся
первый элемент списка и пишется предупреждение в лог.

## 3. Навыки (AuroraSkills)

`Aurora.Skills.Enabled` включает слой, `Aurora.Skills.LogHooks` — построчные
логи на хуках опыта и навыков (по умолчанию выключено). Сами навыки
регистрируются в таблицах `acore_characters` / `acore_world`, а не в
`character_db2.sql` (такого файла в AC нет).

## 4. Интерфейс и логотип (AuroraInterface)

`Aurora::Interface` — серверный заготовок для отправки сообщений клиенту.
Изменения UI, логотипа и `.pak`-ресурсов выполняются на клиенте локально и
не управляются конфигом сервера.

## 5. Настройки

Все кастомизации читаются из `configs/modules/aurora_*.conf` через
`WorldScript::OnStartup`:

```ini
Aurora.Skins.Enabled = 1
Aurora.Skins.Default = "default"
```

Общие правила:

- каждый модуль пишет только свои `Aurora.<Module>.*`;
- список значений через `Aurora::Settings::List()` и `Contains()`;
- числовые значения — через `Bounded()` с явным диапазоном;
- рабочие `.conf` генерирует `scripts/migrate_to_ac.py`;
- после добавления нового `.conf.dist` нужен повторный запуск миграции и
  `cmake -S ... -B ...`, иначе `CONFIG_FILE_LIST` не обновится.

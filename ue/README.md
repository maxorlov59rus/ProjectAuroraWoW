# UE-клиент Aurora

Клиент World of Warcraft WotLK на Unreal Engine 5.8, подключающийся к
серверу Aurora (AzerothCore). Поэтапное портирование: сначала транспорт и
авторизация, затем мир.

## Этап 2: авторизация — реализован SRP6

| Файл | Назначение |
|---|---|
| `Public/Auth/Srp6.h`, `Private/Auth/Srp6.cpp` | SRP6 целиком: x, v, A, u, S, M1, M2, session key. BigInt на байтах LE, без сторонних библиотек |
| `Public/Auth/PacketCodec.h`, `Private/Auth/PacketCodec.cpp` | Сборка/разбор `AUTH_LOGON_CHALLENGE` и `AUTH_LOGON_PROOF`, Pascal-строки, hex |
| `Public/AuroraAuthLibrary.h`, `Private/Auth/AuroraAuthLibrary.cpp` | Полный вход: connect → challenge → SRP6 → proof → проверка M2 |
| `Public/AuroraClient.h` | Точка входа модуля |

`UAuroraAuthLibrary::Login()` возвращает `FAuthSession` с 40-байтовым
session key — он нужен для этапа 3.

## Текущий этап: 1 из 5 — каркас

Готово:
- Проект UE 5.8, модуль `AuroraClient` (`Runtime`, `Default`).
- Зависимости: `Sockets`, `Networking`, `EnhancedInput`, `UMG`.
- Точка входа модуля, целевые конфигурации Game/Editor.
- SRP6 и AuthServer-логин (этап 2).

Не готово (этап 4+): char-select, карты и рендер мира.

## План по этапам

| Этап | Содержание | Состояние |
|---|---|---|
| 1 | Каркас проекта и модуля | Готов |
| 2 | AuthServer: `AUTH_LOGON_CHALLENGE` / `LOGON_PROOF`, SRP6 | Код готов, не проверен на живом сервере |
| 3 | WorldServer: SMSG_AUTH_CHALLENGE, ARC4 + HMAC | Код готов, не проверен на живом сервере |
| 4 | Char-select: SMSG_CHAR_ENUM, CMSG_PLAYER_LOGIN | Готов, собирается |
| 5 | Источники мира: Debug / Procedural / External | Готов, собирается |

## Структура

```
ue/
├── AuroraClient.uproject
├── Source/
│   ├── AuroraClient.Target.cs
│   ├── AuroraClientEditor.Target.cs
│   └── AuroraClient/
│       ├── AuroraClient.Build.cs
│       ├── AuroraClient.cpp
│       ├── Public/
│       │   ├── AuroraClient.h
│       │   ├── AuroraAuthLibrary.h
│       │   └── Auth/{PacketCodec.h, Srp6.h}
│       └── Private/
│           └── Auth/{AuroraAuthLibrary.cpp, PacketCodec.cpp, Srp6.cpp}
└── Config/                 (создаётся редактором при первом запуске)
```

## Сборка

```bat
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" ^
    AuroraClientEditor Win64 Development ^
    -project="<путь>\AuroraClient.uproject"
```

Или открыть `AuroraClient.uproject` — редактор предложит сгенерировать
project files.

## Как проверять авторизацию

1. Поднимите AuthServer (`authserver.exe`) и убедитесь, что он слушает 3724.
2. В редакторе вызовите `AuroraAuthLibrary > Login` с логином и паролем
   из `acore_auth` (тот же, что для игры).
3. `Success` означает, что сервер прислал корректный M2 — пароль и SRP6
   вычислены правильно. Любой другой результат разбирайте по `EAuthResult`.

`Login` блокирующая — в игре вызывайте её из `AsyncTask` или из загрузочного
актора, иначе зависнет игровой поток.

## Важные ограничения

- **Протокол 3.3.5a — бинарный и обфусцированный**: после авторизации
  трафик шифруется ARC4 с ключом из session key, поверх идёт HMAC-SHA1.
- **Карты**: формат ADT/M2/WMO у WoW закрытый и не совпадает с форматами
  UE. Для этапа 5 нужно либо конвертировать данные в UE-формат, либо
  использовать собственные ассеты.
- **Права на ассеты**: модели, текстуры и звуки WoW принадлежат Blizzard.
  Переносить их в UE-проект и распространять нельзя.

## Почему не один большой патч

Ошибка в 1 байте заголовка пакета ломает всё соединение, а отладить это в
UE сложнее, чем в изолированном тесте. Поэтому этап 2 сделан отдельным
слоем `Auth`, который можно поднять против настоящего AuthServer без
запуска игры.

## Этап 5: источники мира

Три варианта отрисовки за одним интерфейсом `IWorldSource`
(`Public/World/WorldSource.h`). Сцена не зависит от способа получения
геометрии — источник отдаёт данные, акторы их показывают.

| Класс | Вариант | Что даёт |
|---|---|---|
| `FDebugWorldSource` | 3 | Плоская сетка 10 ярдов + маркеры сущностей. Проверка, что поток мира живой |
| `FProceduralTerrainSource` | 1 | Рельеф из гладкой косинусной функции, плейсхолдеры вместо моделей |
| `FExternalAssetSource` | 2 | Чтение чанков и сущностей из внешнего каталога по manifest.json |

### Политика лицензий (вариант 2)

`FExternalAssetSource` загружает файл только при наличии в `manifest.json`
записи с допустимой лицензией: `cc0`, `cc-by`, `cc-by-sa`,
`proprietary-own`, `user-owned`. Файлы с `unknown`, `blizzard` или без
поля `license` отбрасываются, и в лог пишется счётчик отклонённых.
Это не формальность: без неё загрузчик стал бы способом распространять
защищённый контент.

Формат манифеста:

```json
{
  "files": [
    { "path": "terrain/azeroth_32_48.json", "map": "azeroth", "license": "cc-by" },
    { "path": "entities/azeroth.json",       "map": "azeroth", "license": "cc-by" }
  ]
}
```
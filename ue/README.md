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

Не готово (этап 3+): пакеты WorldServer, ARC4+HMAC, карты и рендер.

## План по этапам

| Этап | Содержание | Состояние |
|---|---|---|
| 1 | Каркас проекта и модуля | Готов |
| 2 | AuthServer: `AUTH_LOGON_CHALLENGE` / `LOGON_PROOF`, SRP6 | Код готов, не проверен на живом сервере |
| 3 | WorldServer: SMSG_AUTH_CHALLENGE, ARC4 + HMAC | Не начат |
| 4 | Char-select: список персонажей, создание, вход в мир | Не начат |
| 5 | Рендер: тайлы карты, модели, камера, UI | Не начат |

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

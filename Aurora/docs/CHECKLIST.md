# Чек-лист запуска Aurora на AzerothCore

## Вводные

- Вы авторизованный держатель/имеете права на WoW Wotlk-клиент (Blizzard).
- Есть AzerothCore Wotlk в `E:/azerothcore-wotlk-master`.
- MySQL 5.7/8 установлен и запущен.
- Visual Studio 2022 / GCC / Ninja доступен.

---

## 1. Подготовка файлов

- [ ] Убедиться, что `E:/azerothcore-wotlk-master` содержит готовый
      исходный код AzerothCore (CMakeLists.txt, src/, deps/).
- [ ] Скопировать WoW-клиент в `Aurora/externals/WoWClient/`.
- [ ] Создать папку `Aurora/conf/` с конфигами серверов
      (`authserver.conf`, `worldserver.conf`) — скопировать их из AzerothCore
      `conf/dist/` и отредактировать на ваши IP/порт/username.
- [ ] Создать папку `Aurora/databases/` с импортируемыми SQL-схемами
      (Auth.sql, World.sql, Characters.sql — можно скопировать из
      `E:/azerothcore-wotlk-master/databases/` или из установочных скриптов).
- [ ] Запустить `mysql -u root -p < Aurora/databases/Auth.sql` (и World.sql,
      Characters.sql, если есть).

## 2. Билд

- [ ] `cmake --preset windows-ninja` (или `linux-ninja`).
- [ ] `cmake --build --preset windows-ninja` (или `build_linux.sh`).
- [ ] Убедиться, что `build/bin/worldserver.exe` и `build/bin/authserver.exe`
      созданы.

## 3. Инициализация БД

- [ ] `mysql -u root -p < Aurora/databases/Auth.sql`
- [ ] `mysql -u root -p < Aurora/databases/World.sql`
- [ ] Проверить таблицы: `SELECT COUNT(*) FROM world_chartemplate;`

## 4. Запуск серверов

- [ ] Запустить `authserver` в консоли.
  - Ожидать время на авторизацию и рассылку ключа сессии.
- [ ] Запустить `worldserver` в консоли.
  - Ожидать загрузку карты, npc, квест, плагинов.
- [ ] В WoW-клиенте зайти по ip:server:port, ввести логин/пароль.

## 5. Проверка кастомизации Aurora

- [ ] В `src/Aurora/` и `modules/Aurora*/` должны быть зарегистрированы
      перехваты/инфу (нет критических ошибок в логах).
- [ ] В консоли WorldServer должна появляться кастомная строка
      («Aurora greeting», кастомерский тайм-аут и т.п. — настраивается).

---

## Заключение

Если чек-бокс проходит — сервер Aurora готов к массовому запуску.
При возникновении проблем с конфигурацией или логами обращайтесь к
`docs/ARCHITECTURE.md` и `docs/BUILDING.md`.

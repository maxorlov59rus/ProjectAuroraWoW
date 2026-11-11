<?php
declare(strict_types=1);

/**
 * Aurora Web Panel - конфигурация.
 *
 * Скопируйте этот файл в config.local.php и заполните своими значениями.
 * config.local.php в git не попадает (см. .gitignore).
 */

return [
    // ---- Реалмы, отображаемые в панели --------------------------------
    'realms' => [
        [
            'id'          => 1,
            'name'        => 'Aurora Dev',
            'address'     => '127.0.0.1',
            'port'        => 8085,   // WorldServer (worldserver.conf: WorldServerPort)
            'auth_port'   => 3724,   // AuthServer
            'description' => 'Локальный тестовый реалм',
            'icon'        => 'aurora',
        ],
    ],

    // ---- Подключение к acore_auth (для регистрации и списка аккаунтов) --
    'auth_db' => [
        'host'     => '127.0.0.1',
        'port'     => 3306,
        'database' => 'acore_auth',
        'username' => 'acore',
        'password' => '',
    ],

    // ---- SRP6 (используется AzerothCore для хранения паролей) ----------
    // При первом деплое замените на собственную пару из конфигурации AC
    // (AuthSessionKeyLength / SRP6 salt+verifier генерируется на стороне
    //  клиента; здесь нужен только выбор между sha1 и sha256 формой).
    'srp' => [
        'version' => 'sha1',   // 'sha1' (3.3.5a) или 'sha256' (новые сборки AC)
    ],

    // ---- Ссылка на сборку клиента для страницы загрузки -----------------
    'client' => [
        'version'    => '1.0.0',
        'archive'    => '/downloads/aurora-client-1.0.0.zip',
        'size_hint'  => '~1.2 GB',
        'sha256'     => '',
        'release_notes' => [
            'Первый тестовый релиз',
            'Модули: AuroraGameModes, AuroraSkills, AuroraSkins',
        ],
    ],

    // ---- Панель ---------------------------------------------------------
    'panel' => [
        'title'        => 'Aurora',
        'support_email' => 'support@example.com',
        'cache_ttl'    => 15,   // секунд кэширования статуса реалма
        'timezone'     => 'Asia/Yekaterinburg',
        'allow_register' => true,
    ],

    // ---- Логирование ----------------------------------------------------
    'log' => [
        'file'  => __DIR__ . '/../storage/panel.log',
        'level' => 'info', // debug | info | warning | error
    ],
];

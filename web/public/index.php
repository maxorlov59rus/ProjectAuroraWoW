<?php
declare(strict_types=1);

/**
 * Aurora Web Panel — фронт-контроллер.
 *
 * Маршруты:
 *   GET  /                 — дашборд (реалмы + статус)
 *   GET  /register         — форма регистрации
 *   POST /register         — создание аккаунта
 *   GET  /login            — форма входа
 *   POST /login            — проверка пароля по SRP6
 *   POST /logout           — выход
 *   GET  /download         — страница загрузки клиента
 *   GET  /api/status       — JSON со статусом реалмов
 */

require __DIR__ . '/../src/bootstrap.php';

use Aurora\Auth\AccountException;
use Aurora\Auth\AccountManager;
use Aurora\Realm\RealmStatus;

$config = Aurora\config();
$path = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH) ?: '/';

session_start();

/** @var array|null $user */
$user = $_SESSION['user'] ?? null;

switch ($path) {
    case '/':
        render('home', [
            'config' => $config,
            'user'   => $user,
            'realms' => RealmStatus::checkAll($config['realms'], (int) $config['panel']['cache_ttl']),
        ]);
        break;

    case '/api/status':
        header('Content-Type: application/json; charset=utf-8');
        echo json_encode(
            RealmStatus::checkAll($config['realms'], (int) $config['panel']['cache_ttl']),
            JSON_UNESCAPED_UNICODE
        );
        break;

    case '/download':
        render('download', [
            'config' => $config,
            'user'   => $user,
        ]);
        break;

    case '/register':
        if (!$config['panel']['allow_register']) {
            http_response_code(403);
            render('message', ['config' => $config, 'user' => $user,
                'title' => 'Регистрация закрыта', 'text' => 'Приём новых аккаунтов временно приостановлен.']);
            break;
        }

        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            $username = (string) ($_POST['username'] ?? '');
            $password = (string) ($_POST['password'] ?? '');
            $email    = (string) ($_POST['email'] ?? '');

            try {
                $manager = new AccountManager($config['auth_db'], $config['srp']['version']);
                $id = $manager->register($username, $password, $email);
                $_SESSION['user'] = ['id' => $id, 'username' => strtoupper(trim($username))];
                header('Location: /');
                exit;
            } catch (AccountException $e) {
                render('register', ['config' => $config, 'user' => $user,
                    'error' => $e->getMessage(), 'username' => $username, 'email' => $email]);
                break;
            } catch (PDOException $e) {
                render('register', ['config' => $config, 'user' => $user,
                    'error' => 'Нет связи с базой acore_auth. Проверьте config.local.php.',
                    'username' => $username, 'email' => $email]);
                break;
            }
        }

        render('register', ['config' => $config, 'user' => $user]);
        break;

    case '/login':
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            $username = (string) ($_POST['username'] ?? '');
            $password = (string) ($_POST['password'] ?? '');

            try {
                $manager = new AccountManager($config['auth_db'], $config['srp']['version']);
                if ($manager->verify($username, $password)) {
                    $_SESSION['user'] = ['id' => null, 'username' => strtoupper(trim($username))];
                    header('Location: /');
                    exit;
                }
                render('login', ['config' => $config, 'user' => $user,
                    'error' => 'Неверный логин или пароль.', 'username' => $username]);
                break;
            } catch (PDOException $e) {
                render('login', ['config' => $config, 'user' => $user,
                    'error' => 'Нет связи с базой acore_auth. Проверьте config.local.php.',
                    'username' => $username]);
                break;
            }
        }

        render('login', ['config' => $config, 'user' => $user]);
        break;

    case '/logout':
        session_destroy();
        header('Location: /');
        break;

    default:
        http_response_code(404);
        render('message', ['config' => $config, 'user' => $user,
            'title' => '404', 'text' => 'Страница не найдена.']);
}

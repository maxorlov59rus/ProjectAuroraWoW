<?php
declare(strict_types=1);

/**
 * Bootstrap web-панели: автозагрузка, конфиг, вывод视图.
 * Без Composer — панель должна работать на любом shared-хостинге.
 */

spl_autoload_register(static function (string $class): void {
    $prefix = 'Aurora\\';
    if (strncmp($class, $prefix, strlen($prefix)) !== 0) {
        return;
    }

    $relative = substr($class, strlen($prefix));
    $file = __DIR__ . '/' . str_replace('\\', '/', $relative) . '.php';

    if (is_file($file)) {
        require $file;
    }
});

namespace Aurora;

/**
 * Загружает config.local.php поверх config.php.
 *
 * @return array<string, mixed>
 */
function config(): array
{
    static $config = null;

    if ($config !== null) {
        return $config;
    }

    $base = require __DIR__ . '/../config/config.php';
    $localFile = __DIR__ . '/../config/config.local.php';

    if (is_file($localFile)) {
        $local = require $localFile;
        if (is_array($local)) {
            $base = array_replace_recursive($base, $local);
        }
    }

    date_default_timezone_set($base['panel']['timezone'] ?? 'UTC');

    return $config = $base;
}

/**
 * Рендерит шаблон из views/ с общими переменными.
 *
 * @param array<string, mixed> $vars
 */
function render(string $template, array $vars = []): void
{
    $file = __DIR__ . '/../views/' . $template . '.php';
    if (!is_file($file)) {
        http_response_code(500);
        echo 'Template not found: ' . htmlspecialchars($template);
        return;
    }

    extract($vars, EXTR_SKIP);
    require __DIR__ . '/../views/layout.php';
}

/** Экранирование для HTML. */
function e(?string $value): string
{
    return htmlspecialchars((string) $value, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

/** Форматирует unix-время в человекочитаемое. */
function fmt_time(int $ts): string
{
    return date('d.m.Y H:i:s', $ts);
}

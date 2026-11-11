<?php
declare(strict_types=1);

namespace Aurora\Realm;

/**
 * Проверка доступности реалма без подключения к нему как к WoW-клиенту.
 *
 * WorldServer слушает TCP и держит соединение открытым; чтобы не создавать
 * "фантомных" подключений, используется только TCP-handshake с коротким
 * таймаутом и немедленный разрыв.
 */
final class RealmStatus
{
    /** @var array<string, array{value: array, expires: int}> */
    private static array $cache = [];

    /**
     * @param array{address: string, port: int} $realm
     * @return array{online: bool, latency_ms: ?int, checked_at: int, error: ?string}
     */
    public static function check(array $realm, int $cacheTtl = 15): array
    {
        $key = $realm['address'] . ':' . $realm['port'];

        if (isset(self::$cache[$key]) && self::$cache[$key]['expires'] > time()) {
            return self::$cache[$key]['value'];
        }

        $started = microtime(true);
        $error = null;
        $online = false;

        $socket = @fsockopen(
            $realm['address'],
            (int) $realm['port'],
            $errno,
            $errstr,
            2.0
        );

        if (is_resource($socket)) {
            $online = true;
            fclose($socket);
        } else {
            $error = $errstr !== '' ? $errstr : ('errno ' . $errno);
        }

        $value = [
            'online'     => $online,
            'latency_ms' => $online ? (int) round((microtime(true) - $started) * 1000) : null,
            'checked_at' => time(),
            'error'      => $error,
        ];

        self::$cache[$key] = ['value' => $value, 'expires' => time() + $cacheTtl];

        return $value;
    }

    /**
     * Проверяет все реалмы разом.
     *
     * @param array<int, array> $realms
     * @return array<int, array{realm: array, status: array}>
     */
    public static function checkAll(array $realms, int $cacheTtl = 15): array
    {
        $result = [];
        foreach ($realms as $realm) {
            $result[] = ['realm' => $realm, 'status' => self::check($realm, $cacheTtl)];
        }
        return $result;
    }
}

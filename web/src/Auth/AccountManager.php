<?php
declare(strict_types=1);

namespace Aurora\Auth;

use PDO;
use PDOException;

/**
 * Работа с acore_auth: регистрация аккаунта и проверка пароля.
 *
 * AzerothCore 3.3.5a хранит в `account.salt` 32 байта соли, а в
 * `account.verifier` — SRP6 verifier v = g^x mod N, где
 *   x = SHA1(salt | SHA1(USER:PASS))
 * Для сборок с sha256 формой используется другой хеш — см. конфиг.
 *
 * Важно: пароль в открытом виде хранится в байтах строки "USER:PASS" без
 * разделителей, приведённых к верхнему регистру; байты разделителя — это
 * литеральные ':' и '\0', как ожидает AuthServer.
 */
final class AccountManager
{
    private PDO $db;
    private string $srpVersion;

    public function __construct(array $dbConfig, string $srpVersion = 'sha1')
    {
        $dsn = sprintf(
            'mysql:host=%s;port=%d;dbname=%s;charset=utf8mb4',
            $dbConfig['host'],
            (int) $dbConfig['port'],
            $dbConfig['database']
        );

        $this->db = new PDO($dsn, $dbConfig['username'], $dbConfig['password'], [
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
        ]);
        $this->srpVersion = $srpVersion;
    }

    /**
     * Проверяет, что аккаунт существует и пароль совпадает.
     *
     * Проверка идёт по SRP6 verifier: считается x из введённого пароля и
     * сравнивается с v = g^x mod N, сохранённым в БД. Пароль в БД не лежит
     * ни в каком виде, поэтому обычная проверка хеша невозможна.
     */
    public function verify(string $username, string $password): bool
    {
        $username = strtoupper(trim($username));

        $stmt = $this->db->prepare(
            'SELECT salt, verifier FROM account WHERE username = :u LIMIT 1'
        );
        $stmt->execute(['u' => $username]);
        $row = $stmt->fetch();

        if ($row === false) {
            return false;
        }

        $verifier = $this->calculateVerifier($username, $password, $row['salt']);

        // Сравнение константного времени: verifier не секрет, но позволяет
        // избежать лишних ранних выходов и усложняет перебор по таймингам.
        return hash_equals($row['verifier'], $verifier);
    }

    /**
     * Создаёт аккаунт. Возвращает id аккаунта или бросает исключение.
     *
     * @throws AccountException когда логин занят или данные невалидны
     */
    public function register(string $username, string $password, string $email = ''): int
    {
        $username = strtoupper(trim($username));

        if (!preg_match('/^[A-Z0-9]{3,16}$/', $username)) {
            throw new AccountException('Логин: 3–16 символов, только латиница и цифры.');
        }
        if (strlen($password) < 6) {
            throw new AccountException('Пароль должен быть не короче 6 символов.');
        }
        if ($email !== '' && !filter_var($email, FILTER_VALIDATE_EMAIL)) {
            throw new AccountException('Некорректный e-mail.');
        }

        $stmt = $this->db->prepare('SELECT id FROM account WHERE username = :u LIMIT 1');
        $stmt->execute(['u' => $username]);
        if ($stmt->fetch() !== false) {
            throw new AccountException('Такой логин уже занят.');
        }

        $salt = random_bytes(32);
        $verifier = $this->calculateVerifier($username, $password, $salt);

        $insert = $this->db->prepare(
            'INSERT INTO account (username, salt, verifier, email, joindate, expansion)
             VALUES (:u, :s, :v, :e, NOW(), 2)'
        );
        $insert->execute([
            'u' => $username,
            's' => $salt,
            'v' => $verifier,
            'e' => $email,
        ]);

        return (int) $this->db->lastInsertId();
    }

    /**
     * v = g^x mod N, где x = H(salt | H(USER:PASS)).
     * Используются константы SRP6 из AzerothCore.
     */
    private function calculateVerifier(string $username, string $password, string $salt): string
    {
        $hash = $this->srpVersion === 'sha256' ? 'sha256' : 'sha1';

        // Внутренний хеш берётся от USER:PASS без нормализации регистра пароля.
        $inner = hash($hash, $username . ':' . $password, true);
        $x = hash($hash, $salt . $inner, true);

        $g = gmp_init('7', 16);
        $n = gmp_init(
            '894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7',
            16
        );

        // little-endian представление x, как ожидает AzerothCore
        $xLe = gmp_import($x, strlen($x), GMP_LSW_FIRST | GMP_LITTLE_ENDIAN);

        $v = gmp_powm($g, $xLe, $n);

        return gmp_export($v, 32, GMP_MSW_FIRST | GMP_LITTLE_ENDIAN);
    }
}

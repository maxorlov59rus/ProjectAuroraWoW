<?php
/** @var array $config */
/** @var string|null $error */
/** @var string|null $username */
/** @var string|null $email */
?>
<h1>Регистрация</h1>

<?php if (!empty($error)): ?>
    <p class="alert error"><?= \Aurora\e($error) ?></p>
<?php endif; ?>

<form method="post" action="/register" class="form">
    <label>Логин
        <input type="text" name="username" value="<?= \Aurora\e($username ?? '') ?>"
               pattern="[A-Za-z0-9]{3,16}" maxlength="16" required autocomplete="username">
    </label>
    <small>3–16 символов, латиница и цифры. Сохраняется в верхнем регистре.</small>

    <label>Пароль
        <input type="password" name="password" minlength="6" required autocomplete="new-password">
    </label>

    <label>E-mail (необязательно)
        <input type="email" name="email" value="<?= \Aurora\e($email ?? '') ?>" autocomplete="email">
    </label>

    <button type="submit">Создать аккаунт</button>
</form>

<p class="hint">
    Аккаунт создаётся напрямую в <code>acore_auth</code> с SRP6-verifier:
    пароль в открытом виде не сохраняется нигде.
</p>

<?php
/** @var array $config */
/** @var string|null $error */
/** @var string|null $username */
?>
<h1>Вход</h1>

<?php if (!empty($error)): ?>
    <p class="alert error"><?= \Aurora\e($error) ?></p>
<?php endif; ?>

<form method="post" action="/login" class="form">
    <label>Логин
        <input type="text" name="username" value="<?= \Aurora\e($username ?? '') ?>"
               maxlength="16" required autocomplete="username">
    </label>

    <label>Пароль
        <input type="password" name="password" required autocomplete="current-password">
    </label>

    <button type="submit">Войти</button>
</form>

<p class="hint">
    Пароль проверяется по SRP6-verifier из <code>acore_auth</code>.
    Логин тот же, что в игре.
</p>

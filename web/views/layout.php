<?php
/** @var array $config */
/** @var array|null $user */
?>
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title><?= \Aurora\e($config['panel']['title']) ?> — <?= \Aurora\e($title ?? 'Панель') ?></title>
    <link rel="stylesheet" href="/assets/style.css">
</head>
<body>
<header class="topbar">
    <a class="brand" href="/"><?= \Aurora\e($config['panel']['title']) ?></a>
    <nav>
        <a href="/">Дашборд</a>
        <a href="/download">Скачать клиент</a>
        <?php if ($user): ?>
            <span class="who"><?= \Aurora\e($user['username']) ?></span>
            <form method="post" action="/logout" class="inline">
                <button type="submit" class="link">Выйти</button>
            </form>
        <?php else: ?>
            <a href="/login">Вход</a>
            <?php if ($config['panel']['allow_register']): ?>
                <a href="/register">Регистрация</a>
            <?php endif; ?>
        <?php endif; ?>
    </nav>
</header>

<main>
    <?php require $file; ?>
</main>

<footer>
    <?= \Aurora\e($config['panel']['title']) ?> — <?= date('Y') ?> ·
    <a href="mailto:<?= \Aurora\e($config['panel']['support_email']) ?>">Поддержка</a>
</footer>
</body>
</html>

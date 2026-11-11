<?php
/** @var array $config */
/** @var array $realms */
$onlineCount = count(array_filter($realms, static fn($r) => $r['status']['online']));
?>
<h1>Дашборд</h1>

<section class="summary">
    <div class="card">
        <span class="label">Реалмов онлайн</span>
        <span class="value"><?= $onlineCount ?> / <?= count($realms) ?></span>
    </div>
    <div class="card">
        <span class="label">Версия клиента</span>
        <span class="value"><?= \Aurora\e($config['client']['version']) ?></span>
    </div>
</section>

<h2>Реалмы</h2>
<table class="realms">
    <thead>
    <tr>
        <th>Реалм</th>
        <th>Адрес</th>
        <th>Статус</th>
        <th>Пинг</th>
        <th>Проверено</th>
    </tr>
    </thead>
    <tbody>
    <?php foreach ($realms as $entry): ?>
        <?php $realm = $entry['realm']; $status = $entry['status']; ?>
        <tr>
            <td>
                <strong><?= \Aurora\e($realm['name']) ?></strong><br>
                <small><?= \Aurora\e($realm['description'] ?? '') ?></small>
            </td>
            <td><code><?= \Aurora\e($realm['address']) ?>:<?= (int) $realm['port'] ?></code></td>
            <td>
                <?php if ($status['online']): ?>
                    <span class="badge ok">онлайн</span>
                <?php else: ?>
                    <span class="badge down">офлайн</span>
                    <?php if ($status['error']): ?>
                        <br><small><?= \Aurora\e($status['error']) ?></small>
                    <?php endif; ?>
                <?php endif; ?>
            </td>
            <td><?= $status['latency_ms'] !== null ? $status['latency_ms'] . ' мс' : '—' ?></td>
            <td><small><?= \Aurora\fmt_time($status['checked_at']) ?></small></td>
        </tr>
    <?php endforeach; ?>
    </tbody>
</table>

<p class="hint">
    Статус кэшируется на <?= (int) $config['panel']['cache_ttl'] ?> с.
    Для обновления добавьте <code>?t=<?= time() ?></code> к <code>/api/status</code>.
</p>

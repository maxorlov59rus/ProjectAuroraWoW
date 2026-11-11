<?php
/** @var array $config */
$client = $config['client'];
?>
<h1>Скачать клиент</h1>

<section class="card client">
    <div class="row"><span class="label">Версия</span><span class="value"><?= \Aurora\e($client['version']) ?></span></div>
    <div class="row"><span class="label">Размер</span><span class="value"><?= \Aurora\e($client['size_hint']) ?></span></div>
    <?php if (!empty($client['sha256'])): ?>
        <div class="row"><span class="label">SHA-256</span><code class="small"><?= \Aurora\e($client['sha256']) ?></code></div>
    <?php endif; ?>
</section>

<a class="button" href="<?= \Aurora\e($client['archive']) ?>" download>Скачать архив</a>

<?php if (!empty($client['release_notes'])): ?>
    <h2>Что нового</h2>
    <ul>
        <?php foreach ($client['release_notes'] as $note): ?>
            <li><?= \Aurora\e($note) ?></li>
        <?php endforeach; ?>
    </ul>
<?php endif; ?>

<h2>Порядок установки</h2>
<ol>
    <li>Распакуйте архив в отдельную папку (не в системную).</li>
    <li>Запустите <code>launcher.exe</code> из корня папки.</li>
    <li>Введите логин и пароль, созданные на этом сайте.</li>
    <li>Выберите реалм из списка и нажмите «Играть».</li>
</ol>

<p class="hint">
    Лаунчер сверяет контрольные суммы файлов и докачивает изменённые —
    не нужно скачивать архив заново после каждого обновления.
</p>

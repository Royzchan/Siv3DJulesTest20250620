#include <Siv3D.hpp>

void Main()
{
    // 背景色を設定
    Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

    // プレイヤーの初期位置
    Vec2 playerPos{ 100, Scene::Center().y };
    // プレイヤーのサイズ
    Vec2 playerSize{ 40, 60 };
    // プレイヤーの移動速度
    double playerSpeed = 300.0;

    // 弾のリスト
    Array<Vec2> bullets;
    // 弾の速度
    double bulletSpeed = 400.0;
    // 弾のサイズ
    Vec2 bulletSize{ 10, 5 };
    // 発射間隔 (秒)
    double fireRate = 0.25;
    // 前回の発射からの経過時間
    double fireTimer = 0.0;

    // 敵のリスト
    Array<Vec2> enemies;
    // 敵のサイズ
    Vec2 enemySize{ 50, 50 };
    // 敵の速度
    double enemySpeed = 150.0;
    // 敵の出現間隔 (秒)
    double enemySpawnInterval = 1.5;
    // 前回の敵出現からの経過時間
    double enemySpawnTimer = 0.0;

    // プレイヤーのライフ
    int playerLives = 3;
    // 無敵時間 (秒)
    double invincibleTime = 1.0;
    // 無敵タイマー
    double invincibleTimer = 0.0;

    // ゲームオーバー状態
    bool gameOver = false;
    // フォント
    Font font{ 30 };
    Font gameOverFont{ 60, Typeface::Bold };
    Font retryFont{ 20 };

    // スコア
    int score = 0;

    while (System::Update())
    {
        if (!gameOver)
        {
            // 各種タイマー更新
            fireTimer += Scene::DeltaTime();
            enemySpawnTimer += Scene::DeltaTime();
            if (invincibleTimer > 0)
            {
                invincibleTimer -= Scene::DeltaTime();
            }

            // プレイヤーの移動処理
            if (KeyLeft.pressed())
            {
                playerPos.x -= playerSpeed * Scene::DeltaTime();
            }
            if (KeyRight.pressed())
            {
                playerPos.x += playerSpeed * Scene::DeltaTime();
            }
            if (KeyUp.pressed())
            {
                playerPos.y -= playerSpeed * Scene::DeltaTime();
            }
            if (KeyDown.pressed())
            {
                playerPos.y += playerSpeed * Scene::DeltaTime();
            }

            // プレイヤーの移動範囲制限 (画面内)
            playerPos.x = Clamp(playerPos.x, playerSize.x / 2, Scene::Width() - playerSize.x / 2);
            playerPos.y = Clamp(playerPos.y, playerSize.y / 2, Scene::Height() - playerSize.y / 2);

            // 弾の発射処理 (スペースキー)
            if (KeySpace.pressed() && fireTimer >= fireRate)
            {
                // プレイヤーの右側中央から弾を発射
                bullets.emplace_back(playerPos.x + playerSize.x / 2, playerPos.y);
                fireTimer = 0.0; // タイマーリセット
            }

            // 弾の移動
            for (auto& bullet : bullets)
            {
                bullet.x += bulletSpeed * Scene::DeltaTime();
            }

            // 画面外に出た弾を削除
            bullets.remove_if([](const Vec2& b)
            {
                return b.x > Scene::Width();
            });

            // 敵の出現処理
            if (enemySpawnTimer >= enemySpawnInterval)
            {
                // 画面右端のランダムな高さに出現
                enemies.emplace_back(Scene::Width() + enemySize.x / 2, Random(enemySize.y / 2, Scene::Height() - enemySize.y / 2));
                enemySpawnTimer = 0.0; // タイマーリセット
            }

            // 敵の移動
            for (auto& enemy : enemies)
            {
                enemy.x -= enemySpeed * Scene::DeltaTime();
            }

            // 画面外に出た敵を削除
            enemies.remove_if([&](const Vec2& e) // enemySize をキャプチャするために & を追加
            {
                return e.x < -enemySize.x / 2; // 完全に画面外に出たら削除
            });

            // 当たり判定
            // 弾と敵
            for (int i = bullets.size() - 1; i >= 0; --i)
            {
                bool bulletHit = false;
                RectF bulletRect{ Arg::center = bullets[i], bulletSize };
                for (int j = enemies.size() - 1; j >= 0; --j)
                {
                    RectF enemyRect{ Arg::center = enemies[j], enemySize };
                    if (bulletRect.intersects(enemyRect))
                    {
                        bullets.remove_at(i);
                        enemies.remove_at(j);
                        score += 100; // スコア加算
                        bulletHit = true;
                        break; // この弾は消滅したので、他の敵との判定は不要
                    }
                }
                if (bulletHit)
                {
                    // continue;
                }
            }

            // プレイヤーと敵
            if (invincibleTimer <= 0) // 無敵時間中でなければ判定
            {
                RectF playerRect{ Arg::center = playerPos, playerSize };
                for (int i = enemies.size() - 1; i >= 0; --i)
                {
                    RectF enemyRect{ Arg::center = enemies[i], enemySize };
                    if (playerRect.intersects(enemyRect))
                    {
                        enemies.remove_at(i);
                        playerLives--;
                        invincibleTimer = invincibleTime; // 無敵時間開始
                        if (playerLives <= 0)
                        {
                            playerLives = 0; // ライフがマイナスにならないように
                            gameOver = true;
                        }
                        break;
                    }
                }
            }
        }
        else // ゲームオーバー時の処理
        {
            if (KeyEnter.down())
            {
                // ゲーム状態をリセット
                playerPos = Vec2{ 100, Scene::Center().y };
                playerLives = 3;
                bullets.clear();
                enemies.clear();
                invincibleTimer = 0.0;
                fireTimer = 0.0;
                enemySpawnTimer = 0.0;
                score = 0; // スコアリセット
                gameOver = false;
            }
        }

        // --- 描画処理 ---

        // プレイヤー描画
        if (!gameOver)
        {
            if (invincibleTimer > 0 && static_cast<int>(invincibleTimer * 10) % 2 == 0)
            {
                // 点滅（描画しない）
            }
            else
            {
                RectF{ Arg::center = playerPos, playerSize }.draw(Palette::Skyblue);
            }
        }

        // 弾の描画
        for (const auto& bullet : bullets)
        {
            RectF{ Arg::center = bullet, bulletSize }.draw(Palette::Orange);
        }

        // 敵の描画
        for (const auto& enemy : enemies)
        {
            RectF{ Arg::center = enemy, enemySize }.draw(Palette::Red);
        }

        // ライフ表示
        font(U"LIVES: {}"_fmt(playerLives)).draw(10, 10, Palette::Black);
        // スコア表示
        font(U"SCORE: {}"_fmt(score)).draw(10, 40, Palette::Black);

        // ゲームオーバー表示
        if (gameOver)
        {
            gameOverFont(U"GAME OVER").drawAt(Scene::Center(), Palette::Red);
            retryFont(U"PRESS ENTER TO RETRY").drawAt(Scene::Center().x, Scene::Center().y + 50, Palette::Dimgray);
        }
    }
}

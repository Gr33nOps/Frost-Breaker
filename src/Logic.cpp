#include "Logic.hpp"
#include "Collision.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

GameLogic::GameLogic() {
    if (!lavaPickupTexture.loadFromFile("assets/animations/png/LavaPickupPulse.png") &&
        !lavaPickupTexture.loadFromFile("assets/images/LavaPickup.png")) {
        cerr << "Failed to load lava pickup artwork!" << endl;
    }
    lavaPickupTexture.setSmooth(false);
    const auto loadSound = [](SoundBuffer &buffer, Sound &sound, const char *path,
                              float volume = 40) {
        if (!buffer.loadFromFile(path))
            cerr << "Could not load sound: " << path << '\n';
        sound.setBuffer(buffer);
        sound.setVolume(volume);
    };
    loadSound(paddleCollisionBuffer, paddleCollisionSound, "assets/sounds/sfx/paddle-knock.wav");
    loadSound(brickCollisionBuffer, brickCollisionSound, "assets/sounds/sfx/frost-shatter.wav");
    loadSound(rockHitBuffer, rockHitSound, "assets/sounds/sfx/rock-hit.wav", 45);
    loadSound(lavaBuffer, lavaSound, "assets/sounds/sfx/lava-ignite.wav", 45);
    loadSound(launchBuffer, launchSound, "assets/sounds/sfx/launch.wav", 30);
    loadSound(gameOverBuffer, gameOverSound, "assets/sounds/sfx/game-over.wav");
    loadSound(levelCompleteBuffer, levelCompleteSound, "assets/sounds/sfx/level-clear.wav");
    loadSound(startPressBuffer, startPressSound, "assets/sounds/sfx/start.wav");
    loadSound(menuBuffer, menuSound, "assets/sounds/sfx/menu-back.wav");
    loadSound(fallBuffer, fallSound, "assets/sounds/sfx/life-lost.wav");
    loadSound(resumeBuffer, resumeSound, "assets/sounds/sfx/resume.wav");
    loadSound(pauseBuffer, pauseSound, "assets/sounds/sfx/pause.wav");

    if (!backgroundMusic.openFromFile("assets/sounds/music/glacier-run.ogg")) {
        cerr << "Failed to load background music!" << endl;
    }
    if (!menuMusic.openFromFile("assets/sounds/music/snowbound-menu.ogg")) {
        cerr << "Failed to load menu music!" << endl;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(0);
    menuMusic.setLoop(true);
    menuMusic.setVolume(25);
    menuMusic.play();

    loadHighScore();
}

GameLogic::~GameLogic() {
    clearBricks();
}

void GameLogic::moveBall(Paddle &paddle, Ball &ball, float seconds, Vector2f paddleMovement) {
    if (seconds <= 0)
        return;
    ball.updateVisuals(seconds);
    auto position = ball.ballSprite.getPosition();
    position.x = std::clamp(position.x, BALL_RADIUS, PLAYFIELD_WIDTH - BALL_RADIUS);
    position.y = std::max(position.y, BALL_RADIUS);
    const float duration = seconds * 60;
    float remaining = duration;
    struct Hit {
        Collision::Contact contact;
        Bricks *brick = nullptr;
        LavaPickup *pickup = nullptr;
        bool paddle = false;
    };
    std::vector<Hit> hits;
    hits.reserve(4);
    for (int bounce = 0; bounce < 8 && remaining > 0; ++bounce) {
        const float segment = ball.isLava ? std::min(remaining, ball.lavaSeconds * 60) : remaining;
        const auto movement = ball.velocity * segment;
        float first = 1;
        hits.clear();
        const auto remember = [&](Hit hit) {
            if (hit.contact.time < 0 || hit.contact.time > 1)
                return;
            if (hit.contact.time < first - 0.00001f) {
                hits.clear();
                first = hit.contact.time;
            }
            if (std::abs(hit.contact.time - first) <= 0.00001f)
                hits.push_back(hit);
        };
        if (movement.x < 0)
            remember({{(BALL_RADIUS - position.x) / movement.x, {1, 0}, 0}});
        if (movement.x > 0)
            remember({{(PLAYFIELD_WIDTH - BALL_RADIUS - position.x) / movement.x, {-1, 0}, 0}});
        if (movement.y < 0)
            remember({{(BALL_RADIUS - position.y) / movement.y, {0, 1}, 0}});

        auto paddleBounds = paddle.paddleSprite.getGlobalBounds();
        const auto paddleTravel = paddleMovement * (segment / duration);
        paddleBounds.left -= paddleMovement.x * (remaining / duration);
        paddleBounds.top -= paddleMovement.y * (remaining / duration);
        Collision::Contact contact;
        if (ball.velocity.y > 0 &&
            Collision::rectangle(position, movement - paddleTravel, BALL_RADIUS, paddleBounds,
                                 contact) &&
            contact.normal.y < 0 &&
            position.y + movement.y * contact.time <=
                paddleBounds.top + paddleTravel.y * contact.time) {
            remember({contact, nullptr, nullptr, true});
        }
        for (int row = 0; row < rows; ++row)
            for (int col = 0; col < cols; ++col) {
                auto *brick = BRICK[row][col];
                if (!brick || brick->isDestroyed)
                    continue;
                if (Collision::rectangle(position, movement, BALL_RADIUS,
                                         brick->brickSprite.getGlobalBounds(), contact)) {
                    remember({contact, brick});
                }
            }
        for (auto &pickup : lavaPickups) {
            if (pickup.collected)
                continue;
            if (pickup.touches(position, BALL_RADIUS))
                remember({{0, {}, 0}, nullptr, &pickup});
            else if (Collision::rectangle(position, movement, BALL_RADIUS,
                                          pickup.sprite.getGlobalBounds(), contact)) {
                remember({contact, nullptr, &pickup});
            }
        }
        position += movement * first;
        remaining = std::max(0.0f, remaining - segment * first);
        ball.advanceLava(segment * first / 60);
        if (hits.empty())
            continue;

        for (const auto &hit : hits)
            if (hit.pickup) {
                hit.pickup->collected = true;
                ball.addLava();
                effects.impact(PixelImpact::Lava, hit.pickup->sprite.getPosition());
                lavaSound.play();
            }
        Vector2f normal;
        bool hitPaddle = false, hitWall = false;
        for (const auto &hit : hits) {
            if (hit.pickup)
                continue;
            const auto impact = position + hit.contact.normal * (hit.contact.depth - BALL_RADIUS);
            if (hit.brick) {
                if (hit.brick->takeHit()) {
                    score += 10;
                    effects.impact(PixelImpact::Frost, impact);
                    brickCollisionSound.play();
                    if (ball.isLava)
                        continue;
                } else {
                    effects.impact(PixelImpact::Stone, impact, true);
                    rockHitSound.play();
                }
            } else if (hit.paddle) {
                hitPaddle = true;
                effects.impact(PixelImpact::Stone, impact, true);
                paddleCollisionSound.play();
            } else
                hitWall = true;
            normal += hit.contact.normal;
        }
        const float length = std::hypot(normal.x, normal.y);
        if (length == 0)
            continue;
        normal /= length;
        if (hitPaddle) {
            const auto bounds = paddle.paddleSprite.getGlobalBounds();
            const float center =
                bounds.left + bounds.width / 2 - paddleMovement.x * (remaining / duration);
            const float offset =
                std::clamp((position.x - center) / (bounds.width / 2), -1.0f, 1.0f);
            const float angle = offset * 65 * 3.14159265f / 180;
            ball.velocity = {BALL_VELOCITY * std::sin(angle), -BALL_VELOCITY * std::cos(angle)};
            for (const auto &hit : hits) {
                if (hit.brick || hit.pickup || hit.paddle)
                    continue;
                const float approach = Collision::dot(ball.velocity, hit.contact.normal);
                if (approach < 0)
                    ball.velocity -= 2 * approach * hit.contact.normal;
            }
        } else
            ball.velocity -= 2 * Collision::dot(ball.velocity, normal) * normal;
        float separation = 0.001f;
        for (const auto &hit : hits) {
            if (hit.pickup || (ball.isLava && hit.brick && hit.brick->type == BrickType::Ice))
                continue;
            const float alignment = Collision::dot(normal, hit.contact.normal);
            if (alignment > 0)
                separation = std::max(separation, (hit.contact.depth + 0.001f) / alignment);
        }
        position += normal * separation;
        if (hitWall)
            ball.wallCollisionSound.play();
    }
    ball.ballSprite.setPosition(position);
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
}

void GameLogic::checkCollisionWithPickups(Ball &ball) {
    for (auto &pickup : lavaPickups) {
        if (!pickup.touches(ball.ballSprite.getPosition(), BALL_RADIUS))
            continue;
        pickup.collected = true;
        ball.addLava();
        effects.impact(PixelImpact::Lava, pickup.sprite.getPosition());
        lavaSound.play();
    }
}

void GameLogic::initialize(Ball &ball, Paddle &paddle) {
    clearBricks();
    std::vector<Vector2f> pickupPositions;
    Levels::generateLevel(currentLevel, BRICK, rows, cols, pickupPositions);
    for (const auto position : pickupPositions)
        lavaPickups.emplace_back(lavaPickupTexture, position);
    paddle.reset(PLAYFIELD_WIDTH / 2.0f, HEIGHT - 50.0f);
    ball.reset(paddle.paddleSprite.getPosition().x,
               paddle.paddleSprite.getGlobalBounds().top - BALL_RADIUS - 0.01f);
}

void GameLogic::clearBricks() {
    effects.clear();
    lavaPickups.clear();
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COL; j++) {
            delete BRICK[i][j];
            BRICK[i][j] = nullptr;
        }
    }
}

void GameLogic::advanceLevel(Ball &ball, Paddle &paddle) {
    levelCompleteSound.play();
    if (currentLevel < maxLevels) {
        currentLevel++;
        initialize(ball, paddle);
        gameState = STOP;
    } else {
        gameState = WIN;
    }
}

bool GameLogic::areAllBricksDestroyed() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr && BRICK[i][j]->type == BrickType::Ice &&
                !BRICK[i][j]->isDestroyed) {
                return false;
            }
        }
    }
    return true;
}

void GameLogic::resetGame(Ball &ball, Paddle &paddle) {
    currentLevel = 1;
    score = 0;
    lives = 5;
    initialize(ball, paddle);
}

void GameLogic::displayBricks(RenderWindow &window) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr && !BRICK[i][j]->isDestroyed) {
                window.draw(BRICK[i][j]->brickSprite);
            }
        }
    }
    for (const auto &pickup : lavaPickups)
        if (!pickup.collected)
            window.draw(pickup.sprite);
    effects.draw(window);
}

void GameLogic::checkGameOver(Ball &ball, Paddle &paddle) {
    if (ball.getOutOfBound()) {
        ball.setLava(false);
        effects.clear();
        lives--;
        gameState = STOP;
        if (lives <= 0) {
            gameOverSound.play();
            gameState = OVER;
            return;
        }
        fallSound.play();
        paddle.reset(PLAYFIELD_WIDTH / 2.0f, HEIGHT - 50.0f);
        ball.reset(paddle.paddleSprite.getPosition().x,
                   paddle.paddleSprite.getGlobalBounds().top - BALL_RADIUS - 0.01f);
    }
}

void GameLogic::updateAudio(float seconds) {
    if (!audioFocused || seconds <= 0)
        return;
    const auto fade = [seconds](Music &track, float target) {
        const float volume = track.getVolume();
        const float change = 85.0f * seconds;
        const float next =
            volume < target ? std::min(target, volume + change) : std::max(target, volume - change);
        track.setVolume(next);
        if (next > 0 && track.getStatus() != SoundSource::Playing)
            track.play();
        else if (next == 0 && track.getStatus() == SoundSource::Playing)
            track.pause();
    };
    fade(menuMusic, gameState == MENU ? 25.0f : 0.0f);
    fade(backgroundMusic, (gameState == STOP || gameState == PLAYING) ? 28.0f : 0.0f);
}

void GameLogic::update(Paddle &paddle, Ball &ball, float seconds) {
    if (seconds <= 0 || gameState == MENU || gameState == PAUSE)
        return;
    effects.update(seconds);
    if (gameState == WIN || gameState == OVER)
        return;
    for (auto &pickup : lavaPickups)
        pickup.update(seconds);
    const float speed = std::max(PADDLE_VELOCITY, std::hypot(ball.velocity.x, ball.velocity.y));
    const int steps = std::max(1, int(std::ceil(speed * seconds * 60.0f / 2.0f)));
    const float step = seconds / steps;
    for (int i = 0; i < steps; ++i) {
        const auto oldPaddlePosition = paddle.paddleSprite.getPosition();
        paddle.update(step);
        if (gameState == STOP) {
            ball.updateVisuals(step);
            effects.followLava({}, {}, false, step);
            ball.ballSprite.setPosition(paddle.paddleSprite.getPosition().x,
                                        paddle.paddleSprite.getGlobalBounds().top - BALL_RADIUS -
                                            0.01f);
            continue;
        }
        moveBall(paddle, ball, step, paddle.paddleSprite.getPosition() - oldPaddlePosition);
        checkGameOver(ball, paddle);
        if (gameState != PLAYING)
            return;
        effects.followLava(ball.ballSprite.getPosition(), ball.velocity, ball.isLava, step);
    }
    if (gameState == PLAYING && areAllBricksDestroyed())
        advanceLevel(ball, paddle);
}

void GameLogic::inputHandle(Paddle &paddle, Ball &ball, GameDesign &design, RenderWindow &window,
                            const Event &event) {
    if (event.type == Event::KeyPressed) {
        if (event.key.code == Keyboard::Left || event.key.code == Keyboard::Right) {
            paddle.stop();
        } else if (event.key.code == Keyboard::Space) {
            if (gameState == STOP) {
                launchSound.play();
                gameState = PLAYING;
            } else if (gameState == WIN || gameState == OVER) {
                resetGame(ball, paddle);
                startPressSound.play();
                gameState = STOP;
            }
        } else if (event.key.code == Keyboard::Escape) {
            if (gameState == PLAYING) {
                paddle.stop();
                pauseSound.play();
                gameState = PAUSE;
            } else if (gameState == PAUSE) {
                resumeSound.play();
                gameState = PLAYING;
            }
        }
    }
    if (event.type == Event::LostFocus) {
        paddle.stop();
        audioFocused = false;
        backgroundMusic.pause();
        menuMusic.pause();
        backgroundMusic.setVolume(0);
        menuMusic.setVolume(0);
        Sound *sounds[] = {&paddleCollisionSound,
                           &brickCollisionSound,
                           &rockHitSound,
                           &lavaSound,
                           &launchSound,
                           &gameOverSound,
                           &levelCompleteSound,
                           &startPressSound,
                           &menuSound,
                           &fallSound,
                           &pauseSound,
                           &resumeSound,
                           &ball.wallCollisionSound};
        for (auto *sound : sounds)
            sound->stop();
        if (gameState == PLAYING)
            gameState = PAUSE;
    } else if (event.type == Event::GainedFocus)
        audioFocused = true;
    if (event.type == Event::MouseLeft)
        paddle.stop();
    if (!audioFocused)
        return;
    const FloatRect playfield(0, 0, PLAYFIELD_WIDTH, HEIGHT);
    if (event.type == Event::MouseMoved && (gameState == STOP || gameState == PLAYING)) {
        const Vector2f point(float(event.mouseMove.x), float(event.mouseMove.y));
        if (playfield.contains(point))
            paddle.followMouse(point.x);
        else
            paddle.stop();
        return;
    }
    if (event.type != Event::MouseButtonPressed || event.mouseButton.button != Mouse::Left)
        return;
    const Vector2f point(float(event.mouseButton.x), float(event.mouseButton.y));
    if (gameState == STOP) {
        if (playfield.contains(point)) {
            launchSound.play();
            gameState = PLAYING;
        }
        return;
    }
    const auto hit = [&](const Sprite &sprite, float x, float y) {
        const auto bounds = sprite.getLocalBounds();
        const auto scale = sprite.getScale();
        const float width = bounds.width * scale.x, height = bounds.height * scale.y;
        return FloatRect(x - width / 2.0f, y - height / 2.0f, width, height).contains(point);
    };
    if (gameState == MENU) {
        if (hit(design.startButtonSprite, 400, 350)) {
            resetGame(ball, paddle);
            startPressSound.play();
            gameState = STOP;
        } else if (hit(design.quitButtonSprite, 400, 450))
            window.close();
    } else if (gameState == PAUSE) {
        if (hit(design.resumeButtonSprite, 260, 300)) {
            resumeSound.play();
            gameState = PLAYING;
        } else if (hit(design.menuButtonSprite, 260, 400)) {
            menuSound.play();
            gameState = MENU;
        }
    } else if (gameState == OVER || gameState == WIN) {
        if (hit(design.playAgainButtonSprite, 260, 300)) {
            resetGame(ball, paddle);
            startPressSound.play();
            gameState = STOP;
        } else if (hit(design.menuButtonSprite, 260, 400)) {
            menuSound.play();
            gameState = MENU;
        }
    }
}

void GameLogic::saveHighScore() {
    ofstream file("data/highscore.txt");
    file << highScore;
}

void GameLogic::loadHighScore() {
    ifstream file("data/highscore.txt");
    int savedScore = 0;
    if ((file >> savedScore) && savedScore >= 0)
        highScore = std::max(highScore, savedScore);
}

#include "Logic.hpp"
#include <cmath>
#include <fstream>
#include <limits>

int main() {
    std::cout << std::unitbuf;
    Listener::setGlobalVolume(0);
    GameLogic game;
    Ball ball(260, 500);
    Paddle paddle(260, 550);
    GameDesign design;
    RenderWindow window;
    Event mouseMove{};
    mouseMove.type = Event::MouseMoved;
    mouseMove.mouseMove.y = HEIGHT / 2;
    highScore = 1000000;
    std::ofstream report("build/campaign-playtest.csv");
    report << "mode,level,cleared,seconds,lives_lost,frost_remaining,starting_lives,remaining_"
              "lives,pickups_collected\n";
    constexpr float dt = 1.0f / 240.0f;
    constexpr float radians = 3.14159265f / 180.0f;
    int failures = 0;
    for (int mode = 0; mode < 2; ++mode) {
        game.resetGame(ball, paddle);
        const char *modeName = mode == 0 ? "normal" : "pickups";
        for (int level = 1; level <= 10; ++level) {
            const int startingLives = lives;
            int pickupsCollected = 0;
            game.gameState = PLAYING;
            if (mode == 0)
                game.lavaPickups.clear();
            int frame = 0, lost = 0, lastProgress = 0, previousScore = score, returns = 0;
            for (; frame < 240 * 360 && currentLevel == level && game.gameState != WIN &&
                   game.gameState != OVER;
                 ++frame) {
                if (game.gameState == STOP)
                    game.gameState = PLAYING;
                const auto position = ball.ballSprite.getPosition();
                const auto bounds = paddle.paddleSprite.getGlobalBounds();
                float catchX = position.x;
                if (ball.velocity.y > 0) {
                    const float travel =
                        std::max(0.0f, (bounds.top - BALL_RADIUS - position.y) / ball.velocity.y);
                    const float span = PLAYFIELD_WIDTH - 2 * BALL_RADIUS;
                    float folded =
                        std::fmod(position.x - BALL_RADIUS + ball.velocity.x * travel, 2 * span);
                    if (folded < 0)
                        folded += 2 * span;
                    catchX = BALL_RADIUS + (folded <= span ? folded : 2 * span - folded);
                }
                Vector2f target(260, 80);
                float best = std::numeric_limits<float>::max();
                for (int r = 0; r < rows; ++r)
                    for (int c = 0; c < cols; ++c) {
                        const auto *brick = game.BRICK[r][c];
                        if (!brick || brick->isDestroyed || brick->type == BrickType::Rock)
                            continue;
                        const auto b = brick->brickSprite.getGlobalBounds();
                        const Vector2f center(b.left + b.width / 2, b.top + b.height / 2);
                        const float dx = center.x - catchX, dy = bounds.top - center.y;
                        const float distance = dx * dx + dy * dy;
                        if (distance < best) {
                            best = distance;
                            target = center;
                        }
                    }
                float angle = std::atan2(target.x - catchX, bounds.top - BALL_RADIUS - target.y);
                if ((frame - lastProgress) * dt > 4) {
                    const float phase = std::fmod(returns * 0.61803398875f + level * 0.1f, 1.0f);
                    angle = (2 * phase - 1) * 52 * radians;
                }
                const float offset =
                    std::clamp(angle / (65 * radians), -0.85f, 0.85f) * bounds.width / 2;
                const float desired = std::clamp(catchX - offset, bounds.width / 2,
                                                 PLAYFIELD_WIDTH - bounds.width / 2);
                mouseMove.mouseMove.x = int(std::round(desired));
                game.inputHandle(paddle, ball, design, window, mouseMove);
                const int previousLives = lives;
                const bool approachingPaddle = position.y > 500 && ball.velocity.y > 0;
                const bool hadLava = ball.isLava;
                game.update(paddle, ball, dt);
                if (!hadLava && ball.isLava)
                    ++pickupsCollected;
                if (approachingPaddle && ball.velocity.y < 0)
                    ++returns;
                if (score != previousScore) {
                    previousScore = score;
                    lastProgress = frame;
                }
                if (lives < previousLives)
                    lost += previousLives - lives;
            }
            const bool cleared = currentLevel != level || game.gameState == WIN;
            int remaining = 0;
            if (!cleared)
                for (int r = 0; r < rows; ++r)
                    for (int c = 0; c < cols; ++c)
                        if (game.BRICK[r][c] && game.BRICK[r][c]->type == BrickType::Ice &&
                            !game.BRICK[r][c]->isDestroyed)
                            ++remaining;
            report << modeName << ',' << level << ',' << cleared << ',' << frame * dt << ',' << lost
                   << ',' << remaining << ',' << startingLives << ',' << lives << ','
                   << pickupsCollected << '\n';
            std::cout << (cleared ? "PASS " : "FAIL ") << modeName << " level " << level << ": "
                      << frame * dt << " simulated seconds, " << lost << " lives lost, "
                      << remaining << " frost remain, " << lives << " lives left\n";
            if (!cleared) {
                ++failures;
                break;
            }
        }
        const bool finished = game.gameState == WIN && currentLevel == maxLevels && lives > 0;
        std::cout << (finished ? "PASS " : "FAIL ") << modeName << " campaign: " << lives
                  << " of five lives remain\n";
        if (!finished)
            ++failures;
    }
    return failures ? 1 : 0;
}

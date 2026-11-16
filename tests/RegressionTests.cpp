#include "Logic.hpp"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
int failures = 0;
void check(bool ok, const char *name) {
    std::cout << (ok ? "PASS " : "FAIL ") << name << '\n';
    if (!ok)
        ++failures;
}
bool near(float a, float b) {
    return std::abs(a - b) < 0.02f;
}
bool differentPixels(const Image &a, const Image &b) {
    return a.getSize() != b.getSize() ||
           std::memcmp(a.getPixelsPtr(), b.getPixelsPtr(),
                       std::size_t(a.getSize().x) * a.getSize().y * 4) != 0;
}
bool canReachLava(const GameLogic &game, FloatRect orb, bool ignoreFrost) {
    constexpr int step = 5;
    const int width = int((PLAYFIELD_WIDTH - 2 * BALL_RADIUS) / step) + 1;
    const int height = int((HEIGHT - 2 * BALL_RADIUS) / step) + 1;
    std::vector<bool> visited(width * height, false);
    std::vector<int> queue;
    const int start = (height - 1) * width + width / 2;
    queue.push_back(start);
    visited[start] = true;
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const int cell = queue[head], x = cell % width, y = cell / width;
        const Vector2f point(BALL_RADIUS + x * step, BALL_RADIUS + y * step);
        if (std::hypot(point.x - std::clamp(point.x, orb.left, orb.left + orb.width),
                       point.y - std::clamp(point.y, orb.top, orb.top + orb.height)) < BALL_RADIUS)
            return true;
        for (const Vector2i offset :
             {Vector2i(-1, 0), Vector2i(1, 0), Vector2i(0, -1), Vector2i(0, 1)}) {
            const int nx = x + offset.x, ny = y + offset.y;
            if (nx < 0 || nx >= width || ny < 0 || ny >= height || visited[ny * width + nx])
                continue;
            const Vector2f next(BALL_RADIUS + nx * step, BALL_RADIUS + ny * step);
            bool blocked = false;
            for (int r = 0; r < rows && !blocked; ++r)
                for (int c = 0; c < cols; ++c) {
                    const auto *brick = game.BRICK[r][c];
                    if (!brick || brick->isDestroyed ||
                        (ignoreFrost && brick->type == BrickType::Ice))
                        continue;
                    const auto bounds = brick->brickSprite.getGlobalBounds();
                    const float dx =
                        next.x - std::clamp(next.x, bounds.left, bounds.left + bounds.width);
                    const float dy =
                        next.y - std::clamp(next.y, bounds.top, bounds.top + bounds.height);
                    if (dx * dx + dy * dy < BALL_RADIUS * BALL_RADIUS) {
                        blocked = true;
                        break;
                    }
                }
            visited[ny * width + nx] = true;
            if (!blocked)
                queue.push_back(ny * width + nx);
        }
    }
    return false;
}
void checkCollisionRegressions(GameLogic &game, Ball &ball, Paddle &paddle) {
    const auto reset = [&] {
        game.clearBricks();
        rows = 16;
        cols = 13;
        currentLevel = 1;
        lives = 5;
        score = 0;
        highScore = 1000000;
        game.gameState = PLAYING;
        game.BRICK[15][12] = new Bricks(500, 80);
        paddle.updateSize(80, 22);
        paddle.reset(260, 550);
        ball.setLava(false);
    };
    for (int reversed = 0; reversed < 2; ++reversed) {
        reset();
        game.BRICK[0][reversed] = new Bricks(140, 120);
        game.BRICK[0][1 - reversed] = new Bricks(180, 120);
        ball.reset(139, 85);
        ball.velocity = {0, 8};
        game.update(paddle, ball, 1.0f / 60.0f);
        check(near(ball.velocity.x, 0) && near(ball.velocity.y, -8) &&
                  game.BRICK[0][0]->isDestroyed && game.BRICK[0][1]->isDestroyed && score == 20,
              "a centered brick seam breaks both bricks and bounces straight regardless of array "
              "order");
    }
    reset();
    game.BRICK[0][0] = new Bricks(140, 120);
    const float diagonal = BALL_VELOCITY / std::sqrt(2.0f);
    ball.reset(80, 80);
    ball.velocity = {diagonal, diagonal};
    game.update(paddle, ball, 1.0f / 20.0f);
    const float expectedCorner = 100 - (24 - std::sqrt(800.0f) + 20) / std::sqrt(2.0f);
    check(
        near(ball.ballSprite.getPosition().x, expectedCorner) &&
            near(ball.ballSprite.getPosition().y, expectedCorner) &&
            near(ball.velocity.x, -diagonal) && near(ball.velocity.y, -diagonal),
        "round brick corner reflects at the contact point and uses the remaining travel distance");
    reset();
    ball.reset(11, 400);
    ball.velocity = {-8, 0};
    game.update(paddle, ball, 1.0f / 60.0f);
    check(near(ball.ballSprite.getPosition().x, 17) && near(ball.velocity.x, 8),
          "wall bounce keeps the movement remaining after impact");
    reset();
    const auto paddleBounds = paddle.paddleSprite.getGlobalBounds();
    ball.reset(paddleBounds.left + paddleBounds.width + 5, paddleBounds.top - 8);
    ball.velocity = {7, 4};
    game.update(paddle, ball, 0.0001f);
    check(ball.velocity == Vector2f(7, 4),
          "a ball leaving the paddle corner is not caught and bounced again");
    reset();
    ball.reset(306, paddleBounds.top - 11);
    ball.velocity = {0, 8};
    game.update(paddle, ball, 1.0f / 120.0f);
    const float edgeAngle = 65 * 3.14159265f / 180.0f;
    check(near(ball.ballSprite.getPosition().x, 306 + std::sin(edgeAngle)) &&
              near(ball.ballSprite.getPosition().y, paddleBounds.top - 8 - std::cos(edgeAngle)) &&
              ball.velocity.y < 0,
          "paddle corner bounce stays at the rounded contact instead of snapping to the flat top");
    reset();
    paddle.reset(40, 550);
    ball.reset(12, paddleBounds.top - 10 - std::sqrt(48.0f) / 2);
    ball.velocity = {-4, std::sqrt(48.0f)};
    game.update(paddle, ball, 1.0f / 120.0f);
    const float cornerAngle = -0.75f * edgeAngle;
    check(near(ball.velocity.x, -BALL_VELOCITY * std::sin(cornerAngle)) &&
              near(ball.velocity.y, -BALL_VELOCITY * std::cos(cornerAngle)) &&
              ball.ballSprite.getPosition().x >= BALL_RADIUS,
          "a paddle return at the side wall keeps its aim angle and leaves the wall");
    reset();
    paddle.reset(280, 550);
    ball.reset(250, 520);
    ball.velocity = {0, 8};
    game.moveBall(paddle, ball, 1.0f / 40.0f, {40, 0});
    check(near(ball.velocity.x, -BALL_VELOCITY * std::sin(edgeAngle / 2)) &&
              near(ball.velocity.y, -BALL_VELOCITY * std::cos(edgeAngle / 2)),
          "a moving paddle aims from its position at impact rather than its final position");
    reset();
    game.BRICK[0][0] = new Bricks(140, 120);
    ball.reset(80, 92);
    ball.velocity = {8, 0};
    game.update(paddle, ball, 1.0f / 20.0f);
    check(near(ball.velocity.x, 2.24f) && near(ball.velocity.y, -7.68f) &&
              near(ball.ballSprite.getPosition().x, 96.8f) &&
              near(ball.ballSprite.getPosition().y, 82.4f),
          "an oblique round corner reflects along its real normal without changing speed");
    reset();
    ball.reset(260, paddleBounds.top + 2);
    ball.velocity = {0, 8};
    game.update(paddle, ball, 1.0f / 60.0f);
    check(ball.velocity.y > 0, "a ball below the paddle top cannot be rescued from underneath");
    reset();
    game.BRICK[0][0] = new Bricks(180, 140, BrickType::Rock);
    game.BRICK[0][1] = new Bricks(140, 160, BrickType::Rock);
    ball.reset(160, 160);
    ball.velocity = {-diagonal, -diagonal};
    game.update(paddle, ball, 1.0f / 20.0f);
    check(
        near(ball.velocity.x, diagonal) && near(ball.velocity.y, diagonal) &&
            near(ball.ballSprite.getPosition().x, 136 + 24 / std::sqrt(2.0f)) &&
            near(ball.ballSprite.getPosition().y, 136 + 24 / std::sqrt(2.0f)),
        "meeting rock surfaces reflect both axes together without a second impact or lost travel");
    game.resetGame(ball, paddle);
}

void checkCampaignLives(GameLogic &game, Ball &ball, Paddle &paddle, GameDesign &design) {
    game.resetGame(ball, paddle);
    bool livesCarry = true;
    for (int level = 1; level < 10; ++level) {
        if (level % 2 == 0) {
            ball.ballSprite.setPosition(260, 620);
            game.checkGameOver(ball, paddle);
        }
        const int remaining = lives;
        game.advanceLevel(ball, paddle);
        livesCarry = livesCarry && lives == remaining && currentLevel == level + 1;
    }
    check(livesCarry && lives == 1 && currentLevel == 10,
          "one pool of five lives carries through all ten campaign levels");
    score = 250;
    ball.setLava(true);
    ball.ballSprite.setPosition(260, 620);
    game.checkGameOver(ball, paddle);
    check(game.gameState == OVER && lives == 0 && currentLevel == 10 && !ball.isLava,
          "the fifth campaign miss ends the game without refilling lives");
    game.update(paddle, ball, 1.0f / 60.0f);
    check(game.gameState == OVER && lives == 0, "game over cannot continue the old campaign");
    RenderWindow window;
    Event event{};
    event.type = Event::KeyPressed;
    event.key.code = Keyboard::Space;
    game.inputHandle(paddle, ball, design, window, event);
    check(currentLevel == 1 && lives == 5 && score == 0 && game.gameState == STOP && !ball.isLava,
          "restarting after game over begins level one with five lives and zero score");
    ball.ballSprite.setPosition(260, 620);
    game.checkGameOver(ball, paddle);
    for (int level = 1; level <= 10; ++level)
        game.advanceLevel(ball, paddle);
    check(game.gameState == WIN && currentLevel == 10 && lives == 4,
          "finishing all ten stages wins with the lives that remain");
}

void checkMouseControls(GameLogic &game, Ball &ball, Paddle &paddle, GameDesign &design) {
    RenderWindow window;
    const auto moveMouse = [&](int x, int y) {
        Event event{};
        event.type = Event::MouseMoved;
        event.mouseMove = {x, y};
        game.inputHandle(paddle, ball, design, window, event);
    };
    const auto pressKey = [&](Keyboard::Key key) {
        Event event{};
        event.type = Event::KeyPressed;
        event.key.code = key;
        game.inputHandle(paddle, ball, design, window, event);
    };
    game.resetGame(ball, paddle);
    paddle.updateSize(80, 22);
    game.gameState = STOP;
    moveMouse(400, 300);
    game.update(paddle, ball, .1f);
    check(near(paddle.paddleSprite.getPosition().x, 302) &&
              near(ball.ballSprite.getPosition().x, 302),
          "mouse movement steers the paddle at its normal speed and carries the ready ball");
    moveMouse(0, 300);
    paddle.update(2);
    check(near(paddle.paddleSprite.getGlobalBounds().left, 0),
          "mouse control clamps the paddle at the left wall");
    moveMouse(519, 300);
    paddle.update(2);
    const auto bounds = paddle.paddleSprite.getGlobalBounds();
    check(near(bounds.left + bounds.width, PLAYFIELD_WIDTH),
          "mouse control clamps the paddle before the sidebar");

    paddle.reset(260, 550);
    moveMouse(450, 300);
    pressKey(Keyboard::Right);
    paddle.update(.1f);
    check(near(paddle.paddleSprite.getPosition().x, 260),
          "an arrow-key event cancels the old mouse target");
    moveMouse(450, 300);
    moveMouse(700, 300);
    paddle.update(.1f);
    check(near(paddle.paddleSprite.getPosition().x, 260),
          "moving the pointer into the sidebar stops the mouse chase");

    Event click{};
    click.type = Event::MouseButtonPressed;
    click.mouseButton = {Mouse::Left, 700, 300};
    game.inputHandle(paddle, ball, design, window, click);
    check(game.gameState == STOP, "clicking the sidebar does not launch the ball");
    click.mouseButton = {Mouse::Right, 260, 300};
    game.inputHandle(paddle, ball, design, window, click);
    check(game.gameState == STOP, "right-click does not launch the ball");
    click.mouseButton = {Mouse::Left, 260, 300};
    game.inputHandle(paddle, ball, design, window, click);
    check(game.gameState == PLAYING, "left-click in the playfield launches the ready ball");

    game.gameState = PLAYING;
    moveMouse(450, 300);
    pressKey(Keyboard::Escape);
    moveMouse(100, 300);
    pressKey(Keyboard::Escape);
    game.update(paddle, ball, .01f);
    check(near(paddle.paddleSprite.getPosition().x, 260),
          "pause ignores mouse movement and discards an unfinished chase");
    moveMouse(450, 300);
    Event focus{};
    focus.type = Event::LostFocus;
    game.inputHandle(paddle, ball, design, window, focus);
    moveMouse(100, 300);
    focus.type = Event::GainedFocus;
    game.inputHandle(paddle, ball, design, window, focus);
    pressKey(Keyboard::Escape);
    game.update(paddle, ball, .01f);
    check(near(paddle.paddleSprite.getPosition().x, 260),
          "focus loss stops mouse movement until the player moves the pointer again");

    paddle.reset(260, 550);
    ball.reset(270, paddle.paddleSprite.getGlobalBounds().top - 11);
    ball.velocity = {0, 8};
    moveMouse(400, 300);
    game.update(paddle, ball, 1.0f / 60.0f);
    const float angle = (9.125f / 40) * 65 * 3.14159265f / 180;
    check(near(ball.velocity.x, 8 * std::sin(angle)) && near(ball.velocity.y, -8 * std::cos(angle)),
          "mouse-driven paddle collisions use the paddle position at impact");

    game.gameState = MENU;
    paddle.reset(260, 550);
    moveMouse(450, 300);
    paddle.update(.1f);
    check(near(paddle.paddleSprite.getPosition().x, 260),
          "menu pointer movement does not steer the paddle");
    click.mouseButton = {Mouse::Left, 400, 350};
    game.inputHandle(paddle, ball, design, window, click);
    check(game.gameState == STOP, "clicking Start opens ready without also launching the ball");
    pressKey(Keyboard::Space);
    check(game.gameState == PLAYING, "Space still launches after mouse control is added");
    game.resetGame(ball, paddle);
}

int main() {
    std::cout << std::unitbuf;
    Ball ball(260, 500);
    Paddle paddle(260, 550);
    GameLogic game;
    const auto preparePickupLevel = [&] {
        game.resetGame(ball, paddle);
        currentLevel = 3;
        game.initialize(ball, paddle);
    };
    check(game.menuMusic.getStatus() == SoundSource::Playing &&
              game.backgroundMusic.getStatus() == SoundSource::Stopped,
          "title screen starts only the menu soundtrack");
    game.gameState = STOP;
    game.updateAudio(.1f);
    check(game.menuMusic.getStatus() == SoundSource::Playing &&
              game.backgroundMusic.getStatus() == SoundSource::Playing,
          "starting a game crossfades between the two soundtracks");
    game.updateAudio(.5f);
    check(game.menuMusic.getStatus() == SoundSource::Paused &&
              game.backgroundMusic.getStatus() == SoundSource::Playing,
          "finished crossfade pauses the inactive music stream");
    game.gameState = PAUSE;
    game.updateAudio(.5f);
    check(game.backgroundMusic.getStatus() == SoundSource::Paused,
          "pause fades out and pauses gameplay music");
    game.gameState = MENU;
    game.updateAudio(.5f);
    ball.ballSprite.setPosition(5, 400);
    ball.velocity = {8, 0};
    game.moveBall(paddle, ball, 1.0f / 60.0f);
    check(ball.velocity.x > 0 && ball.ballSprite.getGlobalBounds().left >= 0,
          "wall overlap moving inward does not reverse");
    ball.reset(200, 400);
    check(near(ball.ballSprite.getPosition().x, 200), "reset uses requested center");
    check(ball.velocity.y < 0 && near(std::hypot(ball.velocity.x, ball.velocity.y), BALL_VELOCITY),
          "launch goes upward at consistent speed");
    paddle.reset(260, 550);
    check(near(paddle.paddleSprite.getPosition().x, 260), "paddle reset uses requested center");
    paddle.paddleSprite.setPosition(260, 550);
    auto pb = paddle.paddleSprite.getGlobalBounds();
    ball.ballSprite.setPosition(290, pb.top - 9);
    ball.velocity = {0, 8};
    game.moveBall(paddle, ball, 0.000001f);
    check(ball.velocity.x > 0 && ball.velocity.y < 0 &&
              near(std::hypot(ball.velocity.x, ball.velocity.y), BALL_VELOCITY),
          "first paddle edge hit controls angle and keeps speed");
    ball.ballSprite.setPosition(260, pb.top - 9);
    ball.velocity = {3, -7};
    game.moveBall(paddle, ball, 0.000001f);
    check(near(ball.velocity.x, 3) && near(ball.velocity.y, -7),
          "paddle does not hit a ball moving upward");
    paddle.updateSize(120, 30);
    check(near(paddle.paddleSprite.getGlobalBounds().width, 120) &&
              near(paddle.paddleSprite.getGlobalBounds().height, 30),
          "paddle resize uses pixel dimensions");
    game.clearBricks();
    rows = 16;
    cols = 13;
    game.BRICK[0][0] = new Bricks(100, 100);
    auto brick = game.BRICK[0][0]->brickSprite.getGlobalBounds();
    ball.ballSprite.setPosition(brick.left - 9, brick.top - 9);
    ball.velocity = {3, 3};
    game.moveBall(paddle, ball, 0.000001f);
    check(!game.BRICK[0][0]->isDestroyed,
          "circle misses outside brick corner despite square overlap");
    ball.ballSprite.setPosition(brick.left + brick.width / 2, brick.top - 5);
    ball.velocity = {0, 8};
    highScore = 1000000;
    game.moveBall(paddle, ball, 0.000001f);
    check(game.BRICK[0][0]->isDestroyed && ball.velocity.y < 0 &&
              ball.ballSprite.getPosition().y <= brick.top - 10,
          "brick bounce separates ball from surface");
    currentLevel = 7;
    score = 120;
    lives = 1;
    game.resetGame(ball, paddle);
    check(currentLevel == 1 && score == 0 && lives == 5, "new game starts at level one");
    game.clearBricks();
    game.BRICK[20][0] = new Bricks(40, 100);
    game.clearBricks();
    check(game.BRICK[20][0] == nullptr,
          "brick cleanup includes slots outside current level dimensions");
    ball.reset(260, 400);
    check(ball.ballSprite.getPosition().y < 400.1f, "ball reset respects requested height");
    ball.ballSprite.setPosition(260, 400);
    ball.velocity = {3, -4};
    game.moveBall(paddle, ball, 1.0f / 120.0f);
    game.moveBall(paddle, ball, 1.0f / 120.0f);
    check(near(ball.ballSprite.getPosition().x, 263) && near(ball.ballSprite.getPosition().y, 396),
          "movement scales with elapsed time");
    paddle.paddleSprite.setPosition(-50, 550);
    paddle.update(0);
    check(paddle.paddleSprite.getGlobalBounds().left >= 0, "paddle clamps to left boundary");
    paddle.paddleSprite.setPosition(600, 550);
    paddle.update(0);
    check(paddle.paddleSprite.getGlobalBounds().left +
                  paddle.paddleSprite.getGlobalBounds().width <=
              520.01f,
          "paddle clamps to playfield boundary");
    game.initialize(ball, paddle);
    check(ball.ballSprite.getGlobalBounds().top + ball.ballSprite.getGlobalBounds().height <
              paddle.paddleSprite.getGlobalBounds().top,
          "ready ball sits above paddle without overlap");
    std::ifstream originalFile("data/highscore.txt");
    std::string original((std::istreambuf_iterator<char>(originalFile)), {});
    originalFile.close();
    {
        std::ofstream file("data/highscore.txt");
        file << "-99";
    }
    highScore = 250;
    game.loadHighScore();
    check(highScore == 250, "negative saved score is ignored");
    {
        std::ofstream file("data/highscore.txt");
        file << original;
    }
    GameDesign design;
    RenderWindow window;
    Event event{};
    event.type = Event::MouseButtonPressed;
    event.mouseButton.button = Mouse::Left;
    event.mouseButton.x = 400;
    event.mouseButton.y = 350;
    game.gameState = WIN;
    game.inputHandle(paddle, ball, design, window, event);
    check(game.gameState == WIN, "hidden menu start button does not activate on win screen");
    event.mouseButton.x = 260;
    event.mouseButton.y = 300;
    game.inputHandle(paddle, ball, design, window, event);
    check(game.gameState == STOP && currentLevel == 1, "win replay button resets the campaign");
    event.type = Event::KeyPressed;
    event.key.code = Keyboard::Space;
    game.inputHandle(paddle, ball, design, window, event);
    check(game.gameState == PLAYING, "space launches ready ball");
    event.type = Event::LostFocus;
    game.inputHandle(paddle, ball, design, window, event);
    check(game.gameState == PAUSE, "losing focus pauses the game");
    check(game.backgroundMusic.getStatus() != SoundSource::Playing &&
              game.menuMusic.getStatus() != SoundSource::Playing,
          "losing focus also pauses both soundtracks");
    event.type = Event::GainedFocus;
    game.inputHandle(paddle, ball, design, window, event);
    event.type = Event::KeyPressed;
    event.key.code = Keyboard::Escape;
    game.inputHandle(paddle, ball, design, window, event);
    check(game.gameState == PLAYING, "escape resumes paused game");
    preparePickupLevel();
    game.gameState = PLAYING;
    highScore = 1000000;
    const auto levelOnePickup = game.lavaPickups.front().sprite.getPosition();
    ball.reset(levelOnePickup.x, levelOnePickup.y);
    ball.velocity = {0, -1};
    game.update(paddle, ball, 0.0001f);
    check(ball.isLava && ball.velocity.y < 0 && score == 0,
          "touching a protected lava pickup grants power without bouncing or scoring");
    ball.setLava(false);
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(260, 300);
    game.BRICK[0][1] = new Bricks(80, 100);
    ball.ballSprite.setPosition(240, 400);
    ball.velocity = {0, -100};
    score = 0;
    highScore = 1000000;
    game.update(paddle, ball, 1.0f / 30.0f);
    check(game.BRICK[0][0]->isDestroyed && ball.velocity.y > 0 && score == 10,
          "fast ball cannot tunnel through a brick and scores only once");
    game.gameState = PAUSE;
    auto position = ball.ballSprite.getPosition();
    game.update(paddle, ball, 1.0f / 60.0f);
    check(ball.ballSprite.getPosition() == position, "paused physics leaves ball stationary");
    game.gameState = PLAYING;
    lives = 2;
    ball.ballSprite.setPosition(250, 620);
    ball.velocity = {0, 8};
    game.update(paddle, ball, 1.0f / 60.0f);
    check(lives == 1 && game.gameState == STOP, "miss loses exactly one life and waits for launch");
    lives = 1;
    game.gameState = PLAYING;
    ball.ballSprite.setPosition(250, 620);
    game.update(paddle, ball, 1.0f / 60.0f);
    check(lives == 0 && game.gameState == OVER, "last miss ends the game");
    game.clearBricks();
    currentLevel = 10;
    game.gameState = PLAYING;
    ball.reset(260, 450);
    game.levelCompleteSound.stop();
    game.update(paddle, ball, 1.0f / 60.0f);
    check(game.gameState == WIN && currentLevel == 10,
          "last cleared level wins without advancing past campaign");
    check(game.levelCompleteSound.getStatus() == SoundSource::Playing,
          "final campaign clear plays the victory jingle");
    auto strikeRock = [&](Bricks *target) {
        if (!target)
            return;
        const auto bounds = target->brickSprite.getGlobalBounds();
        ball.ballSprite.setPosition(bounds.left + bounds.width / 2, bounds.top - 5);
        ball.velocity = {0, 8};
        game.moveBall(paddle, ball, 0.000001f);
    };
    game.clearBricks();
    score = 0;
    highScore = 1000000;
    game.BRICK[0][0] = new Bricks(120, 120, BrickType::Rock);
    auto *testRock = game.BRICK[0][0];
    const auto intactRockArt = testRock->brickTexture.copyToImage();
    ball.setLava(false);
    ball.reset(100, 100);
    check(game.areAllBricksDestroyed(), "permanent rocks do not prevent level completion");
    check(!testRock->takeHit() && !testRock->isDestroyed,
          "direct damage cannot destroy a permanent rock");
    ball.wallCollisionSound.stop();
    strikeRock(testRock);
    check(ball.wallCollisionSound.getStatus() == SoundSource::Stopped,
          "rock impacts have their own sound rather than the wall tick");
    check(game.rockHitSound.getStatus() == SoundSource::Playing,
          "rock contact plays its impact effect");
    const auto firstCrackArt = testRock->brickTexture.copyToImage();
    check(!differentPixels(intactRockArt, firstCrackArt),
          "permanent rock artwork stays intact after impact");
    check(!testRock->isDestroyed && ball.velocity.y < 0 && score == 0,
          "rock survives its first hit and reflects the frost ball");
    const auto rockBounds = testRock->brickSprite.getGlobalBounds();
    ball.ballSprite.setPosition(rockBounds.left + rockBounds.width / 2, rockBounds.top - 5);
    ball.velocity = {0, -8};
    game.moveBall(paddle, ball, 0.000001f);
    check(!testRock->isDestroyed && score == 0,
          "departing rock overlap does not score or damage the rock");
    for (int hit = 0; hit < 30; ++hit)
        strikeRock(testRock);
    check(!testRock->isDestroyed && ball.velocity.y < 0 && score == 0 && !ball.isLava,
          "repeated frost hits keep rocks permanent and never award points or lava");
    check(!differentPixels(intactRockArt, testRock->brickTexture.copyToImage()),
          "repeated hits never crack rock artwork");
    game.clearBricks();
    score = 0;
    game.BRICK[0][0] = new Bricks(120, 120, BrickType::Rock);
    ball.setLava(true);
    strikeRock(game.BRICK[0][0]);
    check(!game.BRICK[0][0]->isDestroyed && ball.velocity.y < 0,
          "rock survives lava and still bounces");
    for (int hit = 0; hit < 30; ++hit)
        strikeRock(game.BRICK[0][0]);
    check(!game.BRICK[0][0]->isDestroyed && ball.velocity.y < 0 && ball.isLava && score == 0,
          "repeated lava hits reflect without destroying rocks or awarding points");
    check(ball.ballSprite.getTexture() != &ball.ballTexture &&
              near(ball.ballSprite.getGlobalBounds().width, 20),
          "lava retains its dedicated art and collision size");
    preparePickupLevel();
    score = 0;
    const auto pickupPosition = game.lavaPickups.front().sprite.getPosition();
    ball.ballSprite.setPosition(pickupPosition + Vector2f(28, 18));
    ball.velocity = {3, -4};
    game.checkCollisionWithPickups(ball);
    check(!ball.isLava && !game.lavaPickups.front().collected,
          "lava brick misses a ball outside its rounded corner");
    ball.ballSprite.setPosition(pickupPosition + Vector2f(28, 0));
    game.checkCollisionWithPickups(ball);
    check(ball.isLava && game.lavaPickups.front().collected && ball.velocity == Vector2f(3, -4) &&
              score == 0,
          "pickup contact consumes the orb and grants lava without a bounce or points");
    check(game.lavaSound.getStatus() == SoundSource::Playing,
          "collecting a lava orb plays the ignition sound");
    game.lavaSound.stop();
    ball.setLava(false);
    game.checkCollisionWithPickups(ball);
    check(!ball.isLava && game.lavaSound.getStatus() == SoundSource::Stopped && score == 0,
          "a collected orb cannot grant power or replay its sound twice");
    preparePickupLevel();
    ball.ballSprite.setPosition(game.lavaPickups.front().sprite.getPosition());
    game.checkCollisionWithPickups(ball);
    lives = 2;
    ball.ballSprite.setPosition(260, 620);
    game.checkGameOver(ball, paddle);
    check(
        !ball.isLava && lives == 1 && game.lavaPickups.front().collected &&
            ball.ballSprite.getTexture() == &ball.ballTexture,
        "losing a life removes lava power and restores normal artwork without respawning the orb");
    ball.ballSprite.setPosition(game.lavaPickups.front().sprite.getPosition());
    game.checkCollisionWithPickups(ball);
    check(!ball.isLava && game.lavaPickups.size() == 1 && game.lavaPickups.front().collected,
          "losing a life does not respawn the level's one consumed lava brick");
    game.resetGame(ball, paddle);
    check(!ball.isLava && ball.ballSprite.getTexture() == &ball.ballTexture &&
              game.lavaPickups.size() == 1 && !game.lavaPickups.front().collected,
          "a new campaign restores the normal ball and its protected level-one reward");
    preparePickupLevel();
    for (int r = 0; r < MAX_ROW; ++r)
        for (int c = 0; c < MAX_COL; ++c) {
            delete game.BRICK[r][c];
            game.BRICK[r][c] = nullptr;
        }
    check(game.areAllBricksDestroyed(), "an uncollected lava orb does not block level completion");
    game.gameState = PLAYING;
    ball.reset(260, 450);
    game.update(paddle, ball, 0.0001f);
    check(currentLevel == 4 && game.gameState == STOP && !ball.isLava,
          "uncollected optional power cannot prevent advancing a cleared stage");
    preparePickupLevel();
    game.gameState = PLAYING;
    const auto fastPickupPosition = game.lavaPickups.front().sprite.getPosition();
    for (int r = 0; r < MAX_ROW; ++r)
        for (int c = 0; c < MAX_COL; ++c) {
            delete game.BRICK[r][c];
            game.BRICK[r][c] = nullptr;
        }
    game.BRICK[0][0] = new Bricks(80, 80);
    ball.reset(fastPickupPosition.x, fastPickupPosition.y + 60);
    ball.velocity = {0, -100};
    game.update(paddle, ball, 1.0f / 30.0f);
    check(ball.isLava && game.lavaPickups.front().collected,
          "a fast ball cannot tunnel through the circular pickup");
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(120, 120);
    game.BRICK[1][0] = new Bricks(120, 140);
    auto iceBounds = game.BRICK[0][0]->brickSprite.getGlobalBounds();
    score = 0;
    ball.setLava(true);
    ball.ballSprite.setPosition(iceBounds.left + iceBounds.width / 2, iceBounds.top - 5);
    ball.velocity = {0, 8};
    game.moveBall(paddle, ball, 0.000001f);
    check(game.BRICK[0][0]->isDestroyed && ball.velocity.y == 8 && score == 10,
          "lava destroys ice without reversing velocity");
    game.moveBall(paddle, ball, 0.000001f);
    check(score == 10, "destroyed lava-hit brick cannot score twice");
    ball.ballSprite.setPosition(iceBounds.left + iceBounds.width / 2,
                                iceBounds.top + iceBounds.height + 5);
    game.moveBall(paddle, ball, 0.000001f);
    check(game.BRICK[1][0]->isDestroyed && ball.velocity.y == 8 && score == 20,
          "lava continues through successive bricks");
    ball.reset(260, 400);
    check(!ball.isLava && ball.ballSprite.getTexture() == &ball.ballTexture,
          "a new serve starts with a normal ball");
    ball.isLava = true;
    game.gameState = PLAYING;
    lives = 1;
    ball.ballSprite.setPosition(260, 620);
    game.checkGameOver(ball, paddle);
    check(!ball.isLava && game.gameState == OVER &&
              ball.ballSprite.getTexture() == &ball.ballTexture,
          "the final fall removes lava power before game over");
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(120, 120, BrickType::Rock);
    currentLevel = 1;
    score = 70;
    lives = 2;
    game.gameState = PLAYING;
    game.BRICK[0][1] = new Bricks(300, 100, BrickType::Ice);
    ball.reset(260, 450);
    ball.isLava = true;
    game.update(paddle, ball, 1.0f / 60.0f);
    check(currentLevel == 1 && game.gameState == PLAYING,
          "remaining frost prevents completion even though rocks are permanent");
    strikeRock(game.BRICK[0][1]);
    check(game.BRICK[0][0] && !game.BRICK[0][0]->isDestroyed && game.areAllBricksDestroyed(),
          "clearing the final frost completes a stage with intact rocks still present");
    game.update(paddle, ball, 1.0f / 60.0f);
    check(currentLevel == 2 && game.gameState == STOP && score == 80 && !ball.isLava &&
              ball.ballSprite.getTexture() == &ball.ballTexture && game.lavaPickups.size() == 1 &&
              lives == 2,
          "level advance keeps the remaining lives and creates one fresh lava brick");
    ball.ballSprite.setPosition(game.lavaPickups.front().sprite.getPosition());
    game.checkCollisionWithPickups(ball);
    check(ball.isLava && game.lavaPickups.front().collected,
          "the next level's orb must be collected to regain lava power");
    currentLevel = 8;
    game.initialize(ball, paddle);
    check(!ball.isLava && game.lavaPickups.size() == 1,
          "initializing a different level clears lava and places one protected lava brick");
    game.clearBricks();
    currentLevel = maxLevels;
    game.gameState = PLAYING;
    game.BRICK[0][0] = new Bricks(120, 120, BrickType::Rock);
    ball.reset(260, 450);
    ball.setLava(true);
    game.update(paddle, ball, 1.0f / 60.0f);
    check(game.gameState == WIN && ball.isLava && !game.BRICK[0][0]->isDestroyed,
          "the last stage wins while permanent rocks remain and lava stays active");
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(120, 200, BrickType::Ice);
    game.BRICK[1][0] = new Bricks(120, 180, BrickType::Ice);
    game.BRICK[2][0] = new Bricks(120, 160, BrickType::Rock);
    game.BRICK[3][0] = new Bricks(300, 100, BrickType::Ice);
    ball.reset(100, 250);
    ball.setLava(true);
    ball.velocity = {0, -80};
    game.gameState = PLAYING;
    score = 0;
    game.update(paddle, ball, 1.0f / 30.0f);
    check(game.BRICK[0][0]->isDestroyed && game.BRICK[1][0]->isDestroyed &&
              !game.BRICK[2][0]->isDestroyed && !game.BRICK[3][0]->isDestroyed &&
              ball.velocity.y > 0 && ball.isLava && score == 20,
          "fast lava path destroys ice but reflects from permanent rock");
    ball.ballSprite.setPosition(11, 400);
    ball.velocity = {-8, 0};
    game.moveBall(paddle, ball, 1.0f / 60.0f);
    check(ball.velocity.x > 0 && ball.isLava, "lava still bounces off walls");
    const auto lavaPaddleBounds = paddle.paddleSprite.getGlobalBounds();
    ball.ballSprite.setPosition(paddle.paddleSprite.getPosition().x, lavaPaddleBounds.top - 5);
    ball.velocity = {0, 8};
    game.moveBall(paddle, ball, 0.000001f);
    check(ball.velocity.y < 0 && ball.isLava, "lava still bounces off paddle without losing power");
    int rewardLevels = 0;
    for (int level = 1; level <= 10; ++level) {
        ball.setLava(false);
        currentLevel = level;
        game.initialize(ball, paddle);
        int ice = 0, rock = 0, other = 0;
        bool safeBounds = true, openSideEntrances = true;
        const float laneWidth = 2 * BALL_RADIUS + 4;
        const FloatRect leftLane(0, 0, laneWidth, HEIGHT / 2.0f);
        const FloatRect rightLane(PLAYFIELD_WIDTH - laneWidth, 0, laneWidth, HEIGHT / 2.0f);
        for (int r = 0; r < 16; ++r)
            for (int c = 0; c < 13; ++c)
                if (auto *levelBrick = game.BRICK[r][c]) {
                    if (levelBrick->type == BrickType::Rock) {
                        ++rock;
                    } else if (levelBrick->type == BrickType::Ice) {
                        ++ice;
                    } else
                        ++other;
                    const auto bounds = levelBrick->brickSprite.getGlobalBounds();
                    safeBounds = safeBounds && bounds.left >= 0 &&
                                 bounds.left + bounds.width <= PLAYFIELD_WIDTH && bounds.top >= 0 &&
                                 bounds.top + bounds.height <= HEIGHT / 2.0f;
                    openSideEntrances = openSideEntrances && !bounds.intersects(leftLane) &&
                                        !bounds.intersects(rightLane);
                }
        check(ice > 0 && rock > 0 && other == 0,
              "each campaign level retains frost and permanent rock obstacles");
        check(game.lavaPickups.size() == 1,
              "every campaign stage has exactly one protected lava brick");
        if (!game.lavaPickups.empty())
            ++rewardLevels;
        for (const auto &pickup : game.lavaPickups) {
            const auto pos = pickup.sprite.getPosition();
            const auto orbBounds = pickup.sprite.getGlobalBounds();
            check(near(orbBounds.width, 38) && near(orbBounds.height, 18),
                  "lava pickup matches the frost brick canvas");
            bool clearSpace = true, leftFrost = false, rightFrost = false;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c)
                    if (const auto *adjacent = game.BRICK[r][c]) {
                        const auto b = adjacent->brickSprite.getGlobalBounds();
                        if (orbBounds.intersects(b))
                            clearSpace = false;
                        if (adjacent->type == BrickType::Ice) {
                            if (near(b.top + b.height / 2, pos.y) &&
                                near(b.left + b.width / 2 + 40, pos.x))
                                leftFrost = true;
                            if (near(b.top + b.height / 2, pos.y) &&
                                near(b.left + b.width / 2 - 40, pos.x))
                                rightFrost = true;
                        }
                    }
            check(clearSpace && leftFrost && rightFrost && pos.x >= 8 && pos.x <= 512,
                  "lava orbs occupy clear slots between frost bricks");
            check(!canReachLava(game, orbBounds, false),
                  "lava cannot be reached through an open starting path");
            check(canReachLava(game, orbBounds, true),
                  "breaking frost opens a ball-wide route to lava around permanent rocks");
            bool guarded = true;
            for (const Vector2i direction :
                 {Vector2i(-1, 0), Vector2i(1, 0), Vector2i(0, -1), Vector2i(0, 1)}) {
                for (int depth = 1; depth <= 2; ++depth) {
                    const Vector2f expected(pos.x + direction.x * depth * BRICK_WIDTH,
                                            pos.y + direction.y * depth * BRICK_HEIGHT);
                    bool found = false;
                    for (int r = 0; r < rows; ++r)
                        for (int c = 0; c < cols; ++c) {
                            const auto *guardBrick = game.BRICK[r][c];
                            if (!guardBrick || guardBrick->type != BrickType::Ice)
                                continue;
                            const auto b = guardBrick->brickSprite.getGlobalBounds();
                            if (near(b.left + b.width / 2, expected.x) &&
                                near(b.top + b.height / 2, expected.y))
                                found = true;
                        }
                    guarded = guarded && found;
                }
            }
            check(guarded, "two frost layers guard each cardinal approach to a lava reward");
        }
        check(
            safeBounds,
            "campaign bricks stay inside the upper half and leave reaction space above the paddle");
        check(openSideEntrances,
              "campaign formations leave ball-wide side entrances into the upper playfield");
    }
    check(rewardLevels == 10, "all ten story stages provide a protected lava reward");
    ball.setLava(false);
    ball.reset(260, 400);
    ball.velocity = {0, 0};
    const auto initialFrame = ball.ballSprite.getTextureRect();
    ball.updateVisuals(0.13f);
    check(ball.ballSprite.getTextureRect().left != initialFrame.left &&
              near(ball.ballSprite.getGlobalBounds().width, 20) &&
              near(ball.ballSprite.getGlobalBounds().height, 20),
          "stone ball glint advances without changing its collision size");
    ball.reset(260, 400);
    ball.velocity = {0, 0};
    ball.updateVisuals(0.30f);
    const auto longFrame = ball.ballSprite.getTextureRect();
    ball.reset(260, 400);
    ball.velocity = {0, 0};
    for (int i = 0; i < 3; ++i)
        ball.updateVisuals(0.10f);
    check(ball.ballSprite.getTextureRect() == longFrame,
          "sprite animation uses elapsed time rather than update count");
    ball.setLava(true);
    const auto firstLavaFrame = ball.ballSprite.getTextureRect();
    ball.updateVisuals(0.11f);
    check(ball.ballSprite.getTextureRect().left != firstLavaFrame.left &&
              near(ball.ballSprite.getGlobalBounds().width, 20),
          "lava ball flickers inside the original collision canvas");
    ball.setLava(true);
    check(ball.ballSprite.getTextureRect().left != firstLavaFrame.left,
          "reapplying lava mode does not restart its animation");
    ball.reset(260, 400);
    check(ball.ballSprite.getTextureRect().left == 0 && !ball.isLava &&
              ball.ballSprite.getTexture() == &ball.ballTexture,
          "new serve restarts the normal ball animation and removes lava artwork");
    preparePickupLevel();
    auto &glowingPickup = game.lavaPickups.front();
    const auto initialPickupFrame = glowingPickup.sprite.getTextureRect();
    game.gameState = STOP;
    game.update(paddle, ball, 0.18f);
    check(glowingPickup.sprite.getTextureRect().left != initialPickupFrame.left &&
              !glowingPickup.collected && !ball.isLava &&
              near(glowingPickup.sprite.getGlobalBounds().width, 38),
          "idle lava brick pulses without granting power or changing its collision size");
    game.gameState = PAUSE;
    const auto pausedPickupFrame = glowingPickup.sprite.getTextureRect();
    const auto pausedBallFrame = ball.ballSprite.getTextureRect();
    game.update(paddle, ball, 0.21f);
    check(glowingPickup.sprite.getTextureRect() == pausedPickupFrame &&
              ball.ballSprite.getTextureRect() == pausedBallFrame,
          "pause freezes ball and pickup animation");

    RenderWindow effectsWindow(VideoMode(WIDTH, HEIGHT), "Effects verification", Style::None);
    effectsWindow.setVisible(false);
    auto effectImage = [&]() {
        effectsWindow.clear(Color::Transparent);
        game.displayBricks(effectsWindow);
        Texture capture;
        capture.create(WIDTH, HEIGHT);
        capture.update(effectsWindow);
        return capture.copyToImage();
    };
    auto visiblePixels = [](const Image &image, int left, int top, int width, int height) {
        int count = 0;
        for (int y = top; y < top + height; ++y)
            for (int x = left; x < left + width; ++x)
                if (image.getPixel(unsigned(x), unsigned(y)).a)
                    ++count;
        return count;
    };
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(120, 120);
    game.BRICK[0][1] = new Bricks(400, 100);
    score = 0;
    ball.reset(100, 100);
    strikeRock(game.BRICK[0][0]);
    const auto firstBurst = effectImage();
    check(visiblePixels(firstBurst, 70, 85, 65, 55) > 0,
          "destroyed frost leaves visible pixel fragments");
    game.gameState = PAUSE;
    game.update(paddle, ball, 0.15f);
    check(!differentPixels(firstBurst, effectImage()),
          "pause also freezes existing impact particles");
    game.gameState = STOP;
    game.update(paddle, ball, 0.70f);
    check(visiblePixels(effectImage(), 70, 85, 65, 55) == 0,
          "impact fragments expire instead of accumulating");
    game.clearBricks();
    game.BRICK[0][0] = new Bricks(400, 100);
    ball.reset(260, 400);
    ball.setLava(true);
    ball.velocity = {8, 0};
    game.gameState = PLAYING;
    game.update(paddle, ball, 0.12f);
    check(visiblePixels(effectImage(), 245, 380, 90, 40) > 0,
          "moving lava ball leaves a short pixel trail");
    game.initialize(ball, paddle);
    check(visiblePixels(effectImage(), 245, 380, 90, 40) == 0,
          "new level clears the previous lava trail");
    effectsWindow.close();
    checkCollisionRegressions(game, ball, paddle);
    checkCampaignLives(game, ball, paddle, design);
    checkMouseControls(game, ball, paddle, design);
    std::cout << failures << " failures\n";
    return failures ? 1 : 0;
}

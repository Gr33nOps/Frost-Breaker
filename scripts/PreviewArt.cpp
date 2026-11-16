#include "Logic.hpp"
#include <cstdio>
#include <fstream>

int main(int argc, char **argv) {
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Frost Breaker art review", Style::None);
    window.setVisible(false);
    GameDesign design;
    GameLogic game;
    Ball ball(260, 450);
    Paddle paddle(260, 550);
    game.initialize(ball, paddle);
    auto save = [&](const char *path) {
        Texture capture;
        if (!capture.create(WIDTH, HEIGHT))
            return false;
        capture.update(window);
        return capture.copyToImage().saveToFile(path);
    };
    auto gameplay = [&] {
        window.clear();
        design.drawBackground(window);
        game.displayBricks(window);
        window.draw(paddle.paddleSprite);
        window.draw(ball.ballSprite);
        design.drawLevelLogo(window);
        design.drawCurrentLevel(window, currentLevel);
        design.drawLivesLogo(window);
        design.drawLives(window, 5);
        design.drawScoreLogo(window);
        design.drawScore(window, score);
        design.drawHighScoreLogo(window);
        design.drawHighScore(window, 1080);
        design.drawLavaTime(window, ball.lavaSeconds);
    };
    ball.ballSprite.setPosition(260, 440);
    gameplay();
    if (!save("assets/design/gameplay-preview.png"))
        return 1;
    window.clear();
    design.drawMenuScreen(window);
    if (!save("assets/design/menu-preview.png"))
        return 1;
    gameplay();
    design.drawStopOverlay(window);
    if (!save("assets/design/ready-preview.png"))
        return 1;
    gameplay();
    design.drawPausedScreen(window);
    if (!save("assets/design/pause-preview.png"))
        return 1;
    gameplay();
    design.drawGameWinScreen(window);
    if (!save("assets/design/win-preview.png"))
        return 1;
    gameplay();
    design.drawGameOverScreen(window);
    if (!save("assets/design/game-over-preview.png"))
        return 1;
    std::ofstream chapterNames("assets/design/campaign-names.txt");
    if (!chapterNames)
        return 1;
    for (int level = 1; level <= 10; ++level) {
        ball.setLava(false);
        currentLevel = level;
        game.initialize(ball, paddle);
        chapterNames << Levels::name(level) << '\n';
        ball.ballSprite.setPosition(260, 440);
        gameplay();
        char path[100];
        std::snprintf(path, sizeof(path), "assets/design/level-%02d-preview.png", level);
        if (!save(path))
            return 1;
        gameplay();
        design.drawStopOverlay(window);
        std::snprintf(path, sizeof(path), "assets/design/level-%02d-ready-preview.png", level);
        if (!save(path))
            return 1;
    }
    currentLevel = 3;
    ball.setLava(false);
    game.initialize(ball, paddle);
    ball.setLava(true);
    game.lavaPickups.front().collected = true;
    ball.ballSprite.setPosition(220, 300);
    gameplay();
    if (!save("assets/design/lava-preview.png"))
        return 1;
    if (argc > 1 && std::string(argv[1]) == "--animate") {
        auto demoPickup = game.lavaPickups.front();
        demoPickup.collected = false;
        game.clearBricks();
        currentLevel = 1;
        score = 0;
        highScore = 1000000;
        game.lavaPickups.push_back(demoPickup);
        game.lavaPickups.front().sprite.setPosition(220, 252);
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 11; ++c)
                game.BRICK[r][c] = new Bricks(60.0f + c * 40, 140.0f + r * 40);
        game.BRICK[3][0] = new Bricks(140, 260, BrickType::Rock);
        game.BRICK[3][1] = new Bricks(420, 240, BrickType::Rock);
        ball.setLava(false);
        paddle.reset(260, 550);
        ball.reset(180, 350);
        ball.velocity = {3, -7.416198f};
        game.gameState = PLAYING;
        for (int frame = 0; frame < 80; ++frame) {
            paddle.paddleSprite.setPosition(
                std::clamp(ball.ballSprite.getPosition().x, 40.0f, 480.0f), 550);
            for (int step = 0; step < 6; ++step)
                game.update(paddle, ball, 1.0f / 120.0f);
            window.clear();
            design.drawBackground(window);
            game.displayBricks(window);
            window.draw(paddle.paddleSprite);
            window.draw(ball.ballSprite);
            design.drawLevelLogo(window);
            design.drawCurrentLevel(window, currentLevel);
            design.drawLivesLogo(window);
            design.drawLives(window, lives);
            design.drawScoreLogo(window);
            design.drawScore(window, score);
            design.drawHighScoreLogo(window);
            design.drawHighScore(window, 1080);
            char path[100];
            std::snprintf(path, sizeof(path), "build/animation-preview/frame-%03d.png", frame);
            if (!save(path))
                return 1;
        }
    }
    return 0;
}

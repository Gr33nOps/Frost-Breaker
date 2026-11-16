#define NOMINMAX
#include <Windows.h>

#include "Ball.hpp"
#include "Bricks.hpp"
#include "Design.hpp"
#include "Logic.hpp"
#include "Paddle.hpp"
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>

int main() {
    wchar_t executablePath[32768];
    const DWORD length = GetModuleFileNameW(nullptr, executablePath, 32768);
    if (length == 0 || length >= 32768) {
        std::cerr << "Could not locate Frost Breaker resources.\n";
        return 1;
    }
    std::error_code error;
    std::filesystem::current_path(std::filesystem::path(executablePath).parent_path().parent_path(),
                                  error);
    if (error) {
        std::cerr << error.message() << '\n';
        return 1;
    }

    Ball ball(PLAYFIELD_WIDTH / 2.0f, HEIGHT - 80.0f);
    Paddle paddle(PLAYFIELD_WIDTH / 2.0f, HEIGHT - 50.0f);

    RenderWindow window;
    GameLogic gameLogic;
    GameDesign gameDesign;

    window.create(VideoMode(WIDTH, HEIGHT), "Frost Breaker", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    gameLogic.initialize(ball, paddle);

    Clock frameClock;
    float accumulator = 0;
    const float fixedStep = 1.0f / 120.0f;
    while (window.isOpen()) {
        const float elapsed = std::min(frameClock.restart().asSeconds(), 0.1f);
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            gameLogic.inputHandle(paddle, ball, gameDesign, window, event);
        }

        if (!window.isOpen())
            break;
        gameLogic.updateAudio(elapsed);
        if (window.hasFocus() && gameLogic.gameState != MENU && gameLogic.gameState != PAUSE) {
            accumulator += elapsed;
            while (accumulator >= fixedStep) {
                gameLogic.update(paddle, ball, fixedStep);
                accumulator -= fixedStep;
            }
        } else
            accumulator = 0;

        window.clear(Color::White);
        gameDesign.drawBackground(window);
        gameLogic.displayBricks(window);
        window.draw(ball.ballSprite);
        window.draw(paddle.paddleSprite);
        gameDesign.drawLevelLogo(window);
        gameDesign.drawCurrentLevel(window, currentLevel);
        gameDesign.drawScoreLogo(window);
        gameDesign.drawScore(window, score);
        gameDesign.drawLivesLogo(window);
        gameDesign.drawLives(window, lives);
        gameDesign.drawHighScoreLogo(window);
        gameDesign.drawHighScore(window, highScore);

        if (gameLogic.gameState == MENU) {
            gameDesign.drawMenuScreen(window);
        }

        if (gameLogic.gameState == STOP) {
            gameDesign.drawStopOverlay(window);
        }
        if (gameLogic.gameState == PAUSE) {
            gameDesign.drawPausedScreen(window);
        }
        if (gameLogic.gameState == WIN) {
            gameDesign.drawGameWinScreen(window);
        }

        else if (gameLogic.gameState == OVER) {
            gameDesign.drawGameOverScreen(window);
        }

        window.display();
    }

    return 0;
}

#include <SFML/Graphics.hpp>
#include <iostream>
#include "Paddle.hpp"
#include "Ball.hpp"
#include "Bricks.hpp"
#include "Logic.hpp"
#include "Design.hpp"

int main() {
    Ball ball(WIDTH / 2, HEIGHT - 67);
    Paddle paddle(WIDTH / 2, HEIGHT - 50);

    RenderWindow window;
    GameLogic gameLogic;
    GameDesign gameDesign;

    window.create(VideoMode(WIDTH, HEIGHT), "Frost Breaker", Style::Default);
    window.setFramerateLimit(60);
    gameLogic.initialize(ball, paddle);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        gameLogic.inputHandle(paddle, ball, gameDesign, window, gameLogic.gameState);

        if (gameLogic.gameState == PLAYING) {
            ball.update();
            paddle.update();
            gameLogic.checkCollisionWithPaddle(paddle, ball);
            gameLogic.checkCollisionWithBricks(ball);

            // Check if all bricks are destroyed only when necessary
            if (gameLogic.areAllBricksDestroyed()) {
                gameLogic.advanceLevel(ball, paddle);
            }

            gameLogic.checkGameOver(ball, paddle);
        }

        window.clear(Color::White);
        gameDesign.drawBackground(window);
        window.draw(ball.ballSprite);
        window.draw(paddle.paddleSprite);
        gameLogic.displayBricks(window);
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
        // Only draw win/over text if in those states
        if (gameLogic.gameState == WIN) {
            gameDesign.Win(window);
        }

        else if (gameLogic.gameState == OVER) {
            gameDesign.drawGameOverScreen(window);
        }

        window.display();
    }

    return 0;
}
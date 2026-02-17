#pragma once

#include "Paddle.hpp"
#include "Ball.hpp"
#include "Bricks.hpp"
#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include "Levels.hpp"
#include "Design.hpp"

class GameLogic {
public:
    RectangleShape boxShape;
    Bricks* BRICK[MAX_ROW][MAX_COL];
    GameState gameState = MENU;

    SoundBuffer paddleCollisionBuffer;
    SoundBuffer brickCollisionBuffer;
    SoundBuffer startPressBuffer;
    SoundBuffer gameOverBuffer;
    SoundBuffer levelCompleteBuffer;
    SoundBuffer quitBuffer;
    SoundBuffer menuBuffer;
    SoundBuffer pauseBuffer;
    SoundBuffer resumeBuffer;
    SoundBuffer fallBuffer;

    Sound paddleCollisionSound;
    Sound brickCollisionSound;
    Sound startPressSound;
    Sound gameOverSound;
    Sound levelCompleteSound;
    Sound quitSound;
    Sound menuSound;
    Sound resumeSound;
    Sound pauseSound;
    Sound fallSound;

    Music backgroundMusic;

    GameLogic();
    ~GameLogic();

    void checkCollisionWithPaddle(Paddle& paddle, Ball& ball);
    void checkCollisionWithBricks(Ball& ball);
    void initialize(Ball& ball, Paddle& paddle);
    void clearBricks();
    void advanceLevel(Ball& ball, Paddle& paddle);
    bool areAllBricksDestroyed();
    void resetGame(Ball& ball, Paddle& paddle);
    void displayBricks(RenderWindow& window);
    void checkGameOver(Ball& ball, Paddle& paddle);
    void inputHandle(Paddle& paddle, Ball& ball, GameDesign& design, RenderWindow& window, GameState& gamestate);
    void saveHighScore();
    void loadHighScore();
};

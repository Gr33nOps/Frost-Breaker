#pragma once

#include "Ball.hpp"
#include "Bricks.hpp"
#include "Constants.hpp"
#include "Design.hpp"
#include "LavaPickup.hpp"
#include "Levels.hpp"
#include "Paddle.hpp"
#include "PixelEffects.hpp"
#include <SFML/Graphics.hpp>

class GameLogic {
    Texture lavaPickupTexture;

  public:
    RectangleShape boxShape;
    Bricks *BRICK[MAX_ROW][MAX_COL]{};
    GameState gameState = MENU;
    std::vector<LavaPickup> lavaPickups;

    SoundBuffer paddleCollisionBuffer;
    SoundBuffer brickCollisionBuffer;
    SoundBuffer startPressBuffer;
    SoundBuffer gameOverBuffer;
    SoundBuffer levelCompleteBuffer;
    SoundBuffer rockHitBuffer;
    SoundBuffer lavaBuffer;
    SoundBuffer launchBuffer;
    SoundBuffer menuBuffer;
    SoundBuffer pauseBuffer;
    SoundBuffer resumeBuffer;
    SoundBuffer fallBuffer;

    Sound paddleCollisionSound;
    Sound brickCollisionSound;
    Sound startPressSound;
    Sound gameOverSound;
    Sound levelCompleteSound;
    Sound rockHitSound;
    Sound lavaSound;
    Sound launchSound;
    Sound menuSound;
    Sound resumeSound;
    Sound pauseSound;
    Sound fallSound;

    Music backgroundMusic;
    Music menuMusic;

    GameLogic();
    ~GameLogic();

    void moveBall(Paddle &paddle, Ball &ball, float seconds, Vector2f paddleMovement = {});
    void checkCollisionWithPickups(Ball &ball);
    void initialize(Ball &ball, Paddle &paddle);
    void clearBricks();
    void advanceLevel(Ball &ball, Paddle &paddle);
    bool areAllBricksDestroyed();
    void resetGame(Ball &ball, Paddle &paddle);
    void displayBricks(RenderWindow &window);
    void checkGameOver(Ball &ball, Paddle &paddle);
    void inputHandle(Paddle &paddle, Ball &ball, GameDesign &design, RenderWindow &window,
                     const Event &event);
    void update(Paddle &paddle, Ball &ball, float seconds);
    void updateAudio(float seconds);
    void saveHighScore();
    void loadHighScore();

  private:
    PixelEffects effects;
    bool audioFocused = true;
};

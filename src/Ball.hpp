#pragma once

#include <SFML/Graphics.hpp>
#include "Constants.hpp"

class Ball {
public:
    Vector2f velocity;

    Texture ballTexture;
    Sprite ballSprite;
    SoundBuffer wallCollisionBuffer;
    Sound wallCollisionSound;

    Ball(float X, float Y);
    void update();
    bool getOutOfBound();
    void reset(float X, float Y);
};

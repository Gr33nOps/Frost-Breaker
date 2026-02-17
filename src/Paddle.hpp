#pragma once
#ifndef PADDLE_HPP
#define PADDLE_HPP

#include "Constants.hpp"

class Paddle {
public:
    Texture paddleTexture;
    Sprite paddleSprite;

    Vector2f velocity;

    Paddle(float X, float Y);
    void update();
    void updateSize(float newWidth, float newHeight);
    void reset(float X, float Y);
};

#endif // PADDLE_HPP
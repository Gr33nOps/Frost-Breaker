#pragma once

#include "Constants.hpp"

class Paddle {
  public:
    Texture paddleTexture;
    Sprite paddleSprite;

    Vector2f velocity;

    Paddle(float X, float Y);
    void update(float seconds = 1.0f / 60.0f);
    void updateSize(float newWidth, float newHeight);
    void reset(float X, float Y);
    void followMouse(float x);
    void stop();

  private:
    bool mouseControl = false;
    float mouseTarget = 0;
};

#pragma once

#include "Constants.hpp"
#include "PixelAnimation.hpp"
#include <SFML/Graphics.hpp>

class Ball {
  public:
    Vector2f velocity;
    bool isLava = false;
    float lavaSeconds = 0;

    Texture ballTexture;
    Texture lavaTexture;
    Sprite ballSprite;
    SoundBuffer wallCollisionBuffer;
    Sound wallCollisionSound;

    Ball(float X, float Y);
    bool getOutOfBound();
    void reset(float X, float Y);
    void setLava(bool enabled);
    void addLava();
    void advanceLava(float seconds);
    void updateVisuals(float seconds);

  private:
    PixelAnimation animation;
    bool displayedLava = false;
};

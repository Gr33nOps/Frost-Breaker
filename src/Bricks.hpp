#pragma once

#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <string>

enum class BrickType { Ice, Rock };

class Bricks {
  public:
    Texture brickTexture;
    Sprite brickSprite;
    bool isDestroyed = false;
    BrickType type;

    Bricks(float X, float Y, BrickType kind = BrickType::Ice);
    bool takeHit();
};

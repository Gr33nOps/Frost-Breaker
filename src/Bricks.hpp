#pragma once

#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include <string>

class Bricks {
public:
    Texture brickTexture;
    Sprite brickSprite;
    bool isDestroyed = false;

    // Constructor declaration
    Bricks(float X, float Y);
};

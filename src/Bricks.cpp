#include "Bricks.hpp"
#include "Constants.hpp"
#include <iostream>

// Constructor definition
Bricks::Bricks(float X, float Y) {
    if (!brickTexture.loadFromFile("Images/brick.png")) {
        cerr << "Error: Could not load texture from " << "\n";
    }
    brickSprite.setTexture(brickTexture);
    brickSprite.setPosition(X, Y);
    brickSprite.setScale(1,1);
    brickSprite.setOrigin(BRICK_WIDTH, BRICK_HEIGHT);
}

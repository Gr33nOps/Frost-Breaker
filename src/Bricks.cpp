#include "Bricks.hpp"
#include "Constants.hpp"
#include <iostream>

Bricks::Bricks(float X, float Y, BrickType kind) : type(kind) {
    const char *path =
        type == BrickType::Rock ? "assets/images/Rock.png" : "assets/images/Brick.png";
    if (!brickTexture.loadFromFile(path)) {
        cerr << "Could not load brick artwork: " << path << '\n';
    }
    brickSprite.setTexture(brickTexture);
    brickTexture.setSmooth(false);
    brickSprite.setPosition(X, Y);
    brickSprite.setScale(1, 1);
    brickSprite.setOrigin(BRICK_WIDTH, BRICK_HEIGHT);
}

bool Bricks::takeHit() {
    if (type == BrickType::Rock || isDestroyed)
        return false;
    isDestroyed = true;
    return true;
}

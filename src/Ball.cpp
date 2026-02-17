#include "Ball.hpp"
#include "Paddle.hpp"
#include "Constants.hpp"

Ball::Ball(float X, float Y) {
    if (!ballTexture.loadFromFile("Images/Ball.png")) {
        cerr << "Error: Could not load texture from " << "\n";
    }
    ballSprite.setTexture(ballTexture);
    ballSprite.setPosition(X, Y);
    ballSprite.setScale(1,1);
    ballSprite.setOrigin(BALL_RADIUS, BALL_RADIUS);

    if (!wallCollisionBuffer.loadFromFile("sounds/wall collision.wav")) {
        cerr << "Failed to load paddle collision sound!" << endl;
    }
    wallCollisionSound.setBuffer(wallCollisionBuffer);
    wallCollisionSound.setVolume(30); // Set volume to 50% of the maximum
}

void Ball::update() {
    FloatRect bound = ballSprite.getGlobalBounds();

    if (bound.left <= 0 || bound.left + bound.width >= WIDTH - 280) {
        wallCollisionSound.play(); // Play sound on brick collision
        velocity.x = -velocity.x;
    }
    if (bound.top <= 0) {
        wallCollisionSound.play(); // Play sound on brick collision
        velocity.y = -velocity.y;
    }
    ballSprite.move(velocity);
}

bool Ball::getOutOfBound() {
    return ballSprite.getPosition().y - BALL_RADIUS > HEIGHT;
}

void Ball::reset(float X, float Y) {
    ballSprite.setPosition(X - 150, Y);
    velocity = { BALL_VELOCITY, BALL_VELOCITY };
    isFirstCollision = true;
}
#include "Ball.hpp"
#include "Constants.hpp"
#include <algorithm>

Ball::Ball(float X, float Y) {
    if (!ballTexture.loadFromFile("assets/animations/png/BallGlint.png") &&
        !ballTexture.loadFromFile("assets/images/Ball.png")) {
        cerr << "Could not load ball artwork.\n";
    }
    if (!lavaTexture.loadFromFile("assets/animations/png/LavaBallFlicker.png") &&
        !lavaTexture.loadFromFile("assets/images/LavaBall.png")) {
        cerr << "Could not load lava ball artwork.\n";
    }
    ballTexture.setSmooth(false);
    lavaTexture.setSmooth(false);
    ballSprite.setTexture(ballTexture);
    animation.configure(ballTexture, {20, 20}, 0.12f);
    animation.apply(ballSprite);
    ballSprite.setPosition(X, Y);
    ballSprite.setScale(1, 1);
    ballSprite.setOrigin(BALL_RADIUS, BALL_RADIUS);

    if (!wallCollisionBuffer.loadFromFile("assets/sounds/sfx/wall-tick.wav")) {
        cerr << "Could not load wall collision sound." << endl;
    }
    wallCollisionSound.setBuffer(wallCollisionBuffer);
    wallCollisionSound.setVolume(25);
    reset(X, Y);
}

bool Ball::getOutOfBound() {
    return ballSprite.getPosition().y - BALL_RADIUS > HEIGHT;
}

void Ball::reset(float X, float Y) {
    setLava(false);
    animation.configure(isLava ? lavaTexture : ballTexture, {20, 20}, isLava ? 0.10f : 0.12f);
    animation.apply(ballSprite);
    ballSprite.setPosition(X, Y);
    velocity = {BALL_VELOCITY * 0.34202014f, -BALL_VELOCITY * 0.93969262f};
}
void Ball::setLava(bool enabled) {
    isLava = enabled;
    if (!enabled)
        lavaSeconds = 0;
    else if (lavaSeconds <= 0)
        lavaSeconds = 10;
    if (displayedLava != enabled) {
        displayedLava = enabled;
        animation.configure(isLava ? lavaTexture : ballTexture, {20, 20}, isLava ? 0.10f : 0.12f);
    }
    ballSprite.setTexture(isLava ? lavaTexture : ballTexture);
    animation.apply(ballSprite);
    ballSprite.setColor(Color::White);
}

void Ball::addLava() {
    lavaSeconds += 10;
    setLava(true);
}

void Ball::advanceLava(float seconds) {
    if (!isLava || seconds <= 0)
        return;
    lavaSeconds = std::max(0.0f, lavaSeconds - seconds);
    if (lavaSeconds < 0.00001f)
        setLava(false);
}

void Ball::updateVisuals(float seconds) {
    setLava(isLava);
    animation.advance(seconds);
    animation.apply(ballSprite);
}

#include "Paddle.hpp"
#include "Constants.hpp"

Paddle::Paddle(float X, float Y) {
    if (!paddleTexture.loadFromFile("assets/images/Paddle.png")) {
        cerr << "Could not load paddle artwork.\n";
    }

    paddleSprite.setTexture(paddleTexture);
    paddleSprite.setPosition(X, Y);
    paddleSprite.setScale(1, 1);
    paddleSprite.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
}

void Paddle::update(float seconds) {
    const int direction =
        int(Keyboard::isKeyPressed(Keyboard::Right)) - int(Keyboard::isKeyPressed(Keyboard::Left));
    const float oldX = paddleSprite.getPosition().x;
    const float distance = PADDLE_VELOCITY * std::max(seconds, 0.0f) * 60;
    float x = oldX;
    if (direction != 0) {
        mouseControl = false;
        x += direction * distance;
    } else if (mouseControl) {
        x += std::clamp(mouseTarget - x, -distance, distance);
    }
    const float halfWidth = paddleSprite.getGlobalBounds().width / 2.0f;
    x = std::clamp(x, halfWidth, PLAYFIELD_WIDTH - halfWidth);
    paddleSprite.setPosition(x, paddleSprite.getPosition().y);
    velocity = {seconds > 0 ? (x - oldX) / (seconds * 60) : 0.0f, 0.0f};
}

void Paddle::updateSize(float newWidth, float newHeight) {
    const auto size = paddleTexture.getSize();
    if (newWidth <= 0 || newHeight <= 0 || size.x == 0 || size.y == 0)
        return;
    PADDLE_WIDTH = std::min(newWidth, float(PLAYFIELD_WIDTH));
    PADDLE_HEIGHT = newHeight;
    paddleSprite.setOrigin(size.x / 2.0f, size.y / 2.0f);
    paddleSprite.setScale(PADDLE_WIDTH / size.x, PADDLE_HEIGHT / size.y);
    const float halfWidth = PADDLE_WIDTH / 2.0f;
    paddleSprite.setPosition(std::max(halfWidth, std::min(float(PLAYFIELD_WIDTH) - halfWidth,
                                                          paddleSprite.getPosition().x)),
                             paddleSprite.getPosition().y);
}

void Paddle::reset(float X, float Y) {
    paddleSprite.setPosition(X, Y);
    stop();
}

void Paddle::followMouse(float x) {
    const float halfWidth = paddleSprite.getGlobalBounds().width / 2.0f;
    mouseTarget = std::clamp(x, halfWidth, PLAYFIELD_WIDTH - halfWidth);
    mouseControl = true;
}

void Paddle::stop() {
    mouseControl = false;
    velocity = {0, 0};
}

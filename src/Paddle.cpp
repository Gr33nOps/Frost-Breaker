#include "Paddle.hpp"
#include "Constants.hpp"

Paddle::Paddle(float X, float Y) {
    if (!paddleTexture.loadFromFile("Images/Paddle.png")) {
        cerr << "Error: Could not load texture from " << "\n";
    }

    paddleSprite.setTexture(paddleTexture);
    paddleSprite.setPosition(X, Y);
    paddleSprite.setScale(1, 1);
    paddleSprite.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
}

void Paddle::update() {
    FloatRect bound = paddleSprite.getGlobalBounds();

    if (Keyboard::isKeyPressed(Keyboard::Key::Left) && bound.left >= 0) {
        velocity.x = -PADDLE_VELOCITY;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && bound.left + bound.width <= WIDTH - 285) {
        velocity.x = PADDLE_VELOCITY;
    }
    else {
        velocity.x = 0;
    }

    paddleSprite.move(velocity);
}

void Paddle::updateSize(float newWidth, float newHeight) {
    PADDLE_WIDTH = newWidth;
    PADDLE_HEIGHT = newHeight;
    paddleSprite.setScale(Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    paddleSprite.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
}

void Paddle::reset(float X, float Y) {
    paddleSprite.setPosition(X - 150, Y);
}
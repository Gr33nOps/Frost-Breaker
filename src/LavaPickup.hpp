#pragma once
#include "PixelAnimation.hpp"

class LavaPickup {
    PixelAnimation animation;

  public:
    static constexpr int Width = 38;
    static constexpr int Height = 18;
    sf::Sprite sprite;
    bool collected = false;

    LavaPickup(const sf::Texture &texture, sf::Vector2f position) {
        sprite.setTexture(texture);
        sprite.setOrigin(Width / 2.0f, Height / 2.0f);
        sprite.setPosition(position);
        animation.configure(texture, {Width, Height}, 0.14f);
        animation.apply(sprite);
    }
    bool touches(sf::Vector2f center, float radius) const {
        const auto bounds = sprite.getGlobalBounds();
        const float dx = center.x - std::clamp(center.x, bounds.left, bounds.left + bounds.width);
        const float dy = center.y - std::clamp(center.y, bounds.top, bounds.top + bounds.height);
        return !collected && dx * dx + dy * dy < radius * radius;
    }
    void update(float seconds) {
        if (collected)
            return;
        animation.advance(seconds);
        animation.apply(sprite);
    }
};

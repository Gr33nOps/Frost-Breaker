#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

class PixelAnimation {
    sf::Vector2i cell{1, 1};
    int frames = 1;
    float frameSeconds = 0.1f;
    float age = 0;
    bool loop = true;

  public:
    void configure(const sf::Texture &texture, sf::Vector2i size, float duration,
                   bool repeating = true) {
        cell = size;
        frames = std::max(1, int(texture.getSize().x) / cell.x);
        frameSeconds = duration;
        loop = repeating;
        age = 0;
    }
    void advance(float seconds) {
        if (seconds <= 0)
            return;
        const float length = frames * frameSeconds;
        age = loop ? std::fmod(age + seconds, length) : std::min(age + seconds, length);
    }
    void apply(sf::Sprite &sprite) const {
        const int frame = std::min(frames - 1, int((age + 0.000001f) / frameSeconds));
        sprite.setTextureRect({frame * cell.x, 0, cell.x, cell.y});
    }
    bool finished() const {
        return !loop && age >= frames * frameSeconds;
    }
};

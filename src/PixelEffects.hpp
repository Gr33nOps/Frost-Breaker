#pragma once
#include "Constants.hpp"
#include "PixelAnimation.hpp"
#include <array>
#include <vector>

enum class PixelImpact { Frost, Stone, Lava };

class PixelEffects {
    struct Burst {
        sf::Sprite sprite;
        PixelAnimation animation;
    };
    struct TrailPixel {
        sf::Vector2f position;
        float age = 0;
    };
    std::array<sf::Texture, 3> textures;
    std::vector<Burst> bursts;
    std::vector<TrailPixel> trail;
    float trailTime = 0;

  public:
    PixelEffects() {
        const char *paths[] = {"assets/animations/png/IceBurst.png",
                               "assets/animations/png/RockChip.png",
                               "assets/animations/png/LavaBurst.png"};
        for (std::size_t i = 0; i < textures.size(); ++i) {
            if (!textures[i].loadFromFile(paths[i]))
                std::cerr << "Could not load pixel effect: " << paths[i] << '\n';
            textures[i].setSmooth(false);
        }
    }
    void clear() {
        bursts.clear();
        trail.clear();
        trailTime = 0;
    }
    void impact(PixelImpact kind, sf::Vector2f position, bool compact = false) {
        if (bursts.size() >= 48)
            bursts.erase(bursts.begin());
        const auto &texture = textures[std::size_t(kind)];
        Burst burst;
        burst.sprite.setTexture(texture);
        burst.sprite.setOrigin(24, 24);
        burst.sprite.setPosition(std::round(position.x / 2) * 2, std::round(position.y / 2) * 2);
        if (compact)
            burst.sprite.setScale(0.5f, 0.5f);
        burst.animation.configure(texture, {48, 48}, 0.06f, false);
        burst.animation.apply(burst.sprite);
        bursts.push_back(burst);
    }
    void update(float seconds) {
        if (seconds <= 0)
            return;
        for (auto &burst : bursts) {
            burst.animation.advance(seconds);
            burst.animation.apply(burst.sprite);
        }
        bursts.erase(std::remove_if(bursts.begin(), bursts.end(),
                                    [](const Burst &b) { return b.animation.finished(); }),
                     bursts.end());
        for (auto &dot : trail)
            dot.age += seconds;
        trail.erase(std::remove_if(trail.begin(), trail.end(),
                                   [](const TrailPixel &p) { return p.age >= 0.16f; }),
                    trail.end());
    }
    void followLava(sf::Vector2f position, sf::Vector2f velocity, bool enabled, float seconds) {
        if (!enabled) {
            trail.clear();
            trailTime = 0;
            return;
        }
        const float speed = std::hypot(velocity.x, velocity.y);
        if (speed < 0.001f) {
            trailTime = 0;
            return;
        }
        trailTime += seconds;
        if (trailTime < 0.025f)
            return;
        trailTime = std::fmod(trailTime, 0.025f);
        if (trail.size() >= 12)
            trail.erase(trail.begin());
        trail.push_back({position - velocity * (8.0f / speed), 0});
    }
    void draw(sf::RenderWindow &window) const {
        const auto oldView = window.getView();
        sf::View field(sf::FloatRect(0, 0, float(PLAYFIELD_WIDTH), float(HEIGHT)));
        field.setViewport({0, 0, float(PLAYFIELD_WIDTH) / WIDTH, 1});
        window.setView(field);
        for (const auto &dot : trail) {
            const float size = dot.age < 0.08f ? 4.0f : 2.0f;
            sf::RectangleShape pixel({size, size});
            pixel.setOrigin(size / 2, size / 2);
            pixel.setPosition(std::round(dot.position.x / 2) * 2,
                              std::round((dot.position.y - dot.age * 6) / 2) * 2);
            pixel.setFillColor(dot.age < 0.04f   ? sf::Color(255, 179, 79)
                               : dot.age < 0.10f ? sf::Color(246, 108, 56)
                                                 : sf::Color(182, 55, 54));
            window.draw(pixel);
        }
        for (const auto &burst : bursts)
            window.draw(burst.sprite);
        window.setView(oldView);
    }
};

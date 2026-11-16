#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace Collision {
struct Contact {
    float time = 1;
    sf::Vector2f normal;
    float depth = 0;
};

inline float dot(sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.x + a.y * b.y;
}

inline bool circle(sf::Vector2f start, sf::Vector2f movement, float radius, sf::Vector2f center,
                   Contact &contact) {
    const auto delta = start - center;
    const float distance = std::hypot(delta.x, delta.y);
    const float speedSquared = dot(movement, movement);
    if (speedSquared == 0)
        return false;
    if (distance < radius) {
        const auto normal =
            distance > 0.00001f ? delta / distance : -movement / std::sqrt(speedSquared);
        if (dot(movement, normal) >= 0)
            return false;
        contact = {0, normal, radius - distance};
        return true;
    }
    const float approach = dot(delta, movement);
    if (approach >= 0)
        return false;
    const float discriminant =
        approach * approach - speedSquared * (dot(delta, delta) - radius * radius);
    if (discriminant <= 0)
        return false;
    float time = (-approach - std::sqrt(discriminant)) / speedSquared;
    if (time < -0.00001f || time > 1.00001f)
        return false;
    time = std::clamp(time, 0.0f, 1.0f);
    contact = {time, (delta + movement * time) / radius, 0};
    return true;
}

inline bool rectangle(sf::Vector2f start, sf::Vector2f movement, float radius, sf::FloatRect bounds,
                      Contact &contact) {
    const float right = bounds.left + bounds.width;
    const float bottom = bounds.top + bounds.height;
    const auto end = start + movement;
    if (std::max(start.x, end.x) < bounds.left - radius - 0.0001f ||
        std::min(start.x, end.x) > right + radius + 0.0001f ||
        std::max(start.y, end.y) < bounds.top - radius - 0.0001f ||
        std::min(start.y, end.y) > bottom + radius + 0.0001f)
        return false;
    const sf::Vector2f closest(std::clamp(start.x, bounds.left, right),
                               std::clamp(start.y, bounds.top, bottom));
    const auto delta = start - closest;
    const float distance = std::hypot(delta.x, delta.y);
    if (distance < radius) {
        sf::Vector2f normal;
        float depth = radius - distance;
        if (distance > 0.00001f)
            normal = delta / distance;
        else {
            const float gaps[] = {start.x - bounds.left, right - start.x, start.y - bounds.top,
                                  bottom - start.y};
            const sf::Vector2f normals[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
            const auto side = std::min_element(gaps, gaps + 4) - gaps;
            normal = normals[side];
            depth = radius + gaps[side];
        }
        if (dot(movement, normal) >= 0)
            return false;
        contact = {0, normal, depth};
        return true;
    }
    bool found = false;
    const auto face = [&](float time, float across, float low, float high, sf::Vector2f normal) {
        if (time < -0.00001f || time > 1.00001f || across < low - 0.0001f ||
            across > high + 0.0001f)
            return;
        time = std::clamp(time, 0.0f, 1.0f);
        if (!found || time < contact.time) {
            contact = {time, normal, 0};
            found = true;
        }
    };
    if (movement.x > 0) {
        const float time = (bounds.left - radius - start.x) / movement.x;
        face(time, start.y + movement.y * time, bounds.top, bottom, {-1, 0});
    } else if (movement.x < 0) {
        const float time = (right + radius - start.x) / movement.x;
        face(time, start.y + movement.y * time, bounds.top, bottom, {1, 0});
    }
    if (movement.y > 0) {
        const float time = (bounds.top - radius - start.y) / movement.y;
        face(time, start.x + movement.x * time, bounds.left, right, {0, -1});
    } else if (movement.y < 0) {
        const float time = (bottom + radius - start.y) / movement.y;
        face(time, start.x + movement.x * time, bounds.left, right, {0, 1});
    }
    for (const sf::Vector2f corner :
         {sf::Vector2f(bounds.left, bounds.top), sf::Vector2f(right, bounds.top),
          sf::Vector2f(bounds.left, bottom), sf::Vector2f(right, bottom)}) {
        Contact hit;
        if (circle(start, movement, radius, corner, hit) && (!found || hit.time < contact.time)) {
            contact = hit;
            found = true;
        }
    }
    return found;
}
}

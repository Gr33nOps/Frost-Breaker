#pragma once

#include "Bricks.hpp"
#include "Constants.hpp"
#include <vector>

class Levels {
  public:
    static const char *name(int level);
    static const char *story(int level);
    static void generateLevel(int level, Bricks *BRICK[MAX_ROW][MAX_COL], int rows, int cols,
                              std::vector<sf::Vector2f> &pickupPositions);
};

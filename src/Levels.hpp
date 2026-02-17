#pragma once

#include "Bricks.hpp"
#include "Constants.hpp"

class Levels {
public:
    static void generateLevel(int level, Bricks* BRICK[MAX_ROW][MAX_COL], int rows, int cols);
};
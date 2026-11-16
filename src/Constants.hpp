#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
using namespace sf;

const int WIDTH = 800;
const int HEIGHT = 600;
const int PLAYFIELD_WIDTH = 520;

extern float BALL_RADIUS;
extern float BALL_VELOCITY;

extern float PADDLE_WIDTH;
extern float PADDLE_HEIGHT;
extern float PADDLE_VELOCITY;

extern float BRICK_WIDTH;
extern float BRICK_HEIGHT;

extern int score;
extern int lives;

extern int rows;
extern int cols;
extern int currentLevel;
extern int maxLevels;
extern int highScore;
const int MAX_ROW = 30, MAX_COL = 20;

enum GameState { MENU, STOP, PLAYING, PAUSE, WIN, OVER };

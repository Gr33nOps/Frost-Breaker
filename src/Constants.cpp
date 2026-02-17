#include "Constants.hpp"

float BALL_RADIUS = 10.0f;
float BALL_VELOCITY = 8.f;

float PADDLE_WIDTH = 80.0f;
float PADDLE_HEIGHT = 15.0f;
float PADDLE_VELOCITY = 7.f;

float BRICK_WIDTH = 40.0f;
float BRICK_HEIGHT = 20.0f;

int rows = 16;
int cols = 13;
int currentLevel = 1;
int maxLevels = 10;

int score = 0; 
int highScore = 0;
int lives = 5;

bool isFirstCollision = true;
#include "Logic.hpp"
#include "Constants.hpp"
#include "fstream"

GameLogic::GameLogic() {
    // Initialize the brick array
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COL; j++) {
            BRICK[i][j] = nullptr;
        }
    }

    // Struct to simplify sound loading
    struct SoundInfo {
        SoundBuffer& buffer;
        const string filePath;
        const string errorMessage;
    };

    // Sound buffer loading
    const SoundInfo soundFiles[] = {
        {paddleCollisionBuffer, "sounds/paddle collision.wav", "paddle collision sound"},
        {gameOverBuffer, "sounds/game over.wav", "game over sound"},
        {startPressBuffer, "sounds/start press.wav", "start press sound"},
        {levelCompleteBuffer, "sounds/level complete.wav", "level complete sound"},
        {brickCollisionBuffer, "sounds/brick collision.wav", "brick collision sound"},
        {menuBuffer, "sounds/menu.wav", "menu sound"},
        {fallBuffer, "sounds/fall.wav", "fall sound"},
        {resumeBuffer, "sounds/resume.wav", "resume sound"},
        {pauseBuffer, "sounds/pause.wav", "pause sound"}
    };

    for (const auto& sound : soundFiles) {
        if (!sound.buffer.loadFromFile(sound.filePath)) {
            cerr << "Failed to load " << sound.errorMessage << "!" << endl;
        }
    }

    // Load and configure background music
    if (!backgroundMusic.openFromFile("sounds/background musiccc.ogg")) {
        cerr << "Failed to load background music!" << endl;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(10);
    backgroundMusic.play();

    // Assign sound buffers to sound objects
    paddleCollisionSound.setBuffer(paddleCollisionBuffer);
    brickCollisionSound.setBuffer(brickCollisionBuffer);
    gameOverSound.setBuffer(gameOverBuffer);
    levelCompleteSound.setBuffer(levelCompleteBuffer);
    startPressSound.setBuffer(startPressBuffer);
    menuSound.setBuffer(menuBuffer);
    fallSound.setBuffer(fallBuffer);
    pauseSound.setBuffer(pauseBuffer);
    resumeSound.setBuffer(resumeBuffer);

    // Set sound volumes
    const float defaultVolume = 20.0f;
    paddleCollisionSound.setVolume(defaultVolume);
    startPressSound.setVolume(defaultVolume);
    levelCompleteSound.setVolume(defaultVolume);
    gameOverSound.setVolume(defaultVolume);
    menuSound.setVolume(defaultVolume);
    fallSound.setVolume(defaultVolume);
    pauseSound.setVolume(defaultVolume);
    resumeSound.setVolume(defaultVolume);

    // Load the high score
    loadHighScore();
}


GameLogic::~GameLogic() {
    clearBricks();
}

void GameLogic::checkCollisionWithPaddle(Paddle& paddle, Ball& ball) {
    if (ball.ballSprite.getGlobalBounds().intersects(paddle.paddleSprite.getGlobalBounds())) {
        paddleCollisionSound.play(); 
        FloatRect paddleBounds = paddle.paddleSprite.getGlobalBounds();

        if (isFirstCollision) {
            // On first collision, ball goes straight up
            ball.velocity.x = 0;
            ball.velocity.y = -BALL_VELOCITY;
            isFirstCollision = false; // Reset after the first collision
        }
        else {
            // Normal collision handling with angle based on impact
            float ballCenterX = ball.ballSprite.getPosition().x;
            float paddleCenterX = paddleBounds.left + paddleBounds.width / 2;

            float diff = ballCenterX - paddleCenterX;
            float angle = (diff / (paddleBounds.width / 2)) * 60.0f;

            ball.velocity.x = BALL_VELOCITY * sin(angle * (3.14159265 / 180.0f));
            ball.velocity.y = -BALL_VELOCITY * cos(angle * (3.14159265 / 180.0f));
        }
        ball.ballSprite.setPosition(ball.ballSprite.getPosition().x, paddleBounds.top - BALL_RADIUS);
    }
}

void GameLogic::checkCollisionWithBricks(Ball& ball) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr && !BRICK[i][j]->isDestroyed) {
                sf::FloatRect ballBounds = ball.ballSprite.getGlobalBounds();
                sf::FloatRect brickBounds = BRICK[i][j]->brickSprite.getGlobalBounds();

                if (ballBounds.intersects(brickBounds)) {
                    // Calculate overlaps
                    float ball_right = ballBounds.left + ballBounds.width;
                    float ball_bottom = ballBounds.top + ballBounds.height;
                    float brick_right = brickBounds.left + brickBounds.width;
                    float brick_bottom = brickBounds.top + brickBounds.height;

                    float overlap_x = std::min(ball_right, brick_right) - std::max(ballBounds.left, brickBounds.left);
                    float overlap_y = std::min(ball_bottom, brick_bottom) - std::max(ballBounds.top, brickBounds.top);

                    // Determine collision side and reverse velocity
                    if (overlap_x < overlap_y) {
                        ball.velocity.x = -ball.velocity.x;
                    }
                    else {
                        ball.velocity.y = -ball.velocity.y;
                    }

                    // Destroy brick and update score
                    brickCollisionSound.play();
                    BRICK[i][j]->isDestroyed = true;
                    score += 10;
                    if (score > highScore) {
                        highScore = score;
                        saveHighScore();
                    }
                    return; // Handle only one collision per frame
                }
            }
        }
    }
}

void GameLogic::initialize(Ball& ball, Paddle& paddle) {
    clearBricks();
    Levels::generateLevel(currentLevel, BRICK, rows, cols);
    ball.reset(WIDTH / 2, HEIGHT - 67);
    paddle.reset(WIDTH / 2, HEIGHT - 50);

}

void GameLogic::clearBricks() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr) {
                delete BRICK[i][j];
                BRICK[i][j] = nullptr;
            }
        }
    }
}

void GameLogic::advanceLevel(Ball& ball, Paddle& paddle) {
    if (currentLevel < maxLevels) {
        levelCompleteSound.play();
        lives = 5;
        currentLevel++;
        initialize(ball, paddle);
        gameState = STOP;
    }
    else {
        gameState = WIN;
    }
}

bool GameLogic::areAllBricksDestroyed() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr && !BRICK[i][j]->isDestroyed) {
                return false;
            }
        }
    }
    return true;
}

void GameLogic::resetGame(Ball& ball, Paddle& paddle) {
    score = 0;
    lives = 5;
    initialize(ball, paddle);
}

void GameLogic::displayBricks(RenderWindow& window) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (BRICK[i][j] != nullptr && !BRICK[i][j]->isDestroyed) {
                window.draw(BRICK[i][j]->brickSprite);
            }
        }
    }
}

void GameLogic::checkGameOver(Ball& ball, Paddle& paddle) {
    if (ball.getOutOfBound()) {
        fallSound.play();
        lives--;
        cout << lives << "\n";
        gameState = STOP;
        if (lives <= 0) {
            gameOverSound.play();
            gameState = OVER;
            return;
        }
        ball.reset(WIDTH / 2, HEIGHT - 67);
        paddle.reset(WIDTH / 2, HEIGHT - 50);
    }
}

void GameLogic::inputHandle(Paddle& paddle, Ball& ball, GameDesign& design, RenderWindow& window, GameState& gameState) {
    static bool wasSpacePressed = false;
    static bool wasEscPressed = false;

    // Mouse input handling for MENU and OVER states
    if (gameState == MENU || gameState == OVER) {
        if (Mouse::isButtonPressed(Mouse::Left)) {
            Vector2i mousePos = Mouse::getPosition(window);
            FloatRect startButtonBounds = design.startButtonSprite.getGlobalBounds();
            FloatRect quitButtonBounds = design.quitButtonSprite.getGlobalBounds();
            FloatRect playAgainButtonBounds = design.playAgainButtonSprite.getGlobalBounds();
            FloatRect menuButtonBounds = design.menuButtonSprite.getGlobalBounds();

            // Start button
            if (startButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                gameState = STOP; // Ready to play
                startPressSound.play();
                resetGame(ball, paddle);
            }

            // Quit button
            if (quitButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                window.close(); // Exit the game
            }

            // Try Again button in OVER state
            if (gameState == OVER && playAgainButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                startPressSound.play();
                resetGame(ball, paddle);
                gameState = STOP;
            }
            if (gameState == OVER && menuButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                menuSound.play();
                resetGame(ball, paddle);
                gameState = MENU;
            }
        }
    }

    // Keyboard input handling for SPACE
    if (Keyboard::isKeyPressed(Keyboard::Space)) {
        if (!wasSpacePressed) {
            if (gameState == STOP) {
                gameState = PLAYING; // Start the game
            }
            else if (gameState == WIN || gameState == OVER) {
                resetGame(ball, paddle);
                gameState = STOP; // Prepare for a new round
            }
            wasSpacePressed = true;
        }
    }
    else {
        wasSpacePressed = false;
    }

    // Keyboard input handling for ESC
    if (Keyboard::isKeyPressed(Keyboard::Escape)) {
        if (!wasEscPressed) {
            if (gameState == PLAYING) {
                pauseSound.play();
                gameState = PAUSE; // Pause the game
            }
            else if (gameState == PAUSE) {
                resumeSound.play();
                gameState = PLAYING; // Resume the game
            }
            wasEscPressed = true;
        }
    }
    else {
        wasEscPressed = false;
    }

    // Paddle movement handling in STOP state
    if (gameState == STOP) {
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            if (paddle.paddleSprite.getPosition().x - PADDLE_WIDTH / 2 >= 0) {
                paddle.paddleSprite.move(-PADDLE_VELOCITY, 0);
                ball.ballSprite.setPosition(paddle.paddleSprite.getPosition().x, ball.ballSprite.getPosition().y);
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            if (paddle.paddleSprite.getPosition().x + PADDLE_WIDTH / 2 <= WIDTH - 285) {
                paddle.paddleSprite.move(PADDLE_VELOCITY, 0);
                ball.ballSprite.setPosition(paddle.paddleSprite.getPosition().x, ball.ballSprite.getPosition().y);
            }
        }
    }

    // Mouse input handling for PAUSE state buttons
    if (gameState == PAUSE) {
        if (Mouse::isButtonPressed(Mouse::Left)) {
            Vector2i mousePos = Mouse::getPosition(window);
            FloatRect resumeButtonBounds = design.resumeButtonSprite.getGlobalBounds();
            FloatRect quitButtonBounds = design.quitButtonSprite.getGlobalBounds();
            FloatRect menuButtonBounds = design.menuButtonSprite.getGlobalBounds(); 

            if (resumeButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                resumeSound.play();
                gameState = PLAYING; // Resume the game
            }

            if (menuButtonBounds.contains(static_cast<Vector2f>(mousePos))) {
                menuSound.play();
                gameState = MENU; // Exit the game
            }
        }
    }
}

// Save high score
void GameLogic::saveHighScore() {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

// Load high score
void GameLogic::loadHighScore() {
    ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

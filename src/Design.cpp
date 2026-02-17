#include "Logic.hpp"
#include "Design.hpp"
#include "Constants.hpp"

GameDesign::GameDesign() {
    // Load font
    if (!font.loadFromFile("Font/mini_pixel-7.ttf")) {
        cerr << "Error loading font!" << endl;
    }
    loadTexture(overlayTexture, "Images/overlay.png", "background image");
    loadTexture(menuTexture, "Images/menu background.png", "background image");
    loadTexture(gameLogoTexture, "Images/Logo.png", "background image");
    loadTexture(gameLogoTexture2, "Images/Logo.png", "background image");
    loadTexture(startButtonTexture, "Images/start.png", "button image");
    loadTexture(quitButtonTexture, "Images/quit.png", "button image");
    loadTexture(resumeButtonTexture, "Images/resume.png", "button image");
    loadTexture(playAgainButtonTexture, "Images/play again.png", "button image");
    loadTexture(menuButtonTexture, "Images/menu.png", "menu image");
    loadTexture(startButtonTexture2, "Images/start2.png", "button image");
    loadTexture(quitButtonTexture2, "Images/quit2.png", "button image");
    loadTexture(resumeButtonTexture2, "Images/resume2.png", "button image");
    loadTexture(playAgainButtonTexture2, "Images/play again2.png", "button image");
    loadTexture(menuButtonTexture2, "Images/menu2.png", "menu image");
    loadTexture(backgroundTexture, "Images/background.png", "background image");


    setSpriteTexture();
    loadLivesTextures();
    setupTextElements();
}

void GameDesign::loadTexture(Texture& texture, const string& filePath, const string& errorMessage) {
    if (!texture.loadFromFile(filePath)) {
        cerr << "Error loading " << errorMessage << " from: " << filePath << endl;
    }
}

void GameDesign::setSpriteTexture() {
    overlaySprite.setTexture(overlayTexture);
    menuSprite.setTexture(menuTexture);
    gameLogoSprite.setTexture(gameLogoTexture);
    gameLogoSprite2.setTexture(gameLogoTexture2);
    startButtonSprite.setTexture(startButtonTexture);
    quitButtonSprite.setTexture(quitButtonTexture);
    resumeButtonSprite.setTexture(resumeButtonTexture);
    playAgainButtonSprite.setTexture(playAgainButtonTexture);
    menuButtonSprite.setTexture(menuButtonTexture);
    startButtonSprite2.setTexture(startButtonTexture2);
    quitButtonSprite2.setTexture(quitButtonTexture2);
    resumeButtonSprite2.setTexture(resumeButtonTexture2);
    playAgainButtonSprite2.setTexture(playAgainButtonTexture2);
    menuButtonSprite2.setTexture(menuButtonTexture2);
    backgroundSprite.setTexture(backgroundTexture);
}

void GameDesign::loadLivesTextures() {
    for (int i = 0; i < 5; ++i) {
        string fileName = "Images/" + to_string(i + 1) + " lives.png";
        if (!livesTexture[i].loadFromFile(fileName)) {
            cerr << "Error loading " << i + 1 << " lives!" << endl;
        }
        livesSprite[i].setTexture(livesTexture[i]);
        livesSprite[i].setPosition(550, 260);
    }
}

void GameDesign::setupTextElements() {
    setupText(Score, 50, Color::White);
    setupText(levelLogo, 50, Color::Cyan);
    setupText(levelText, 50, Color::White);
    setupText(scoreText, 50, Color::Cyan);
    setupText(textLives, 50, Color::Cyan);
    setupText(gameOverText, 100, Color::White);
    setupText(gameWinText, 50, Color::Black);
    setupText(highScoreLogo, 50, Color::Cyan);
    setupText(highScore, 50, Color::White);
    setupText(readyText, 100, Color::White);
    setupText(pausedText, 100, Color::White);
}

void GameDesign::setupText(Text& textElement, int size, Color color) {
    textElement.setFont(font);
    textElement.setCharacterSize(size);
    textElement.setFillColor(color);
}

void GameDesign::drawTextCentered(RenderWindow& window, Text& textElement, const string& text, float x, float y) {
    textElement.setString(text);
    FloatRect bounds = textElement.getLocalBounds();
    textElement.setOrigin(bounds.width / 2, bounds.height / 2);
    textElement.setPosition(x, y);
    window.draw(textElement);
}

void GameDesign::drawBackground(RenderWindow& window) {
    window.draw(backgroundSprite);
}

void GameDesign::drawLevelLogo(RenderWindow& window) {
    drawTextCentered(window, levelLogo, "Level", 660, 100);
}

void GameDesign::drawCurrentLevel(RenderWindow& window, int currentLevel) {
    drawTextCentered(window, levelText, to_string(currentLevel), 660, 150);
}

void GameDesign::drawScoreLogo(RenderWindow& window) {
    drawTextCentered(window, scoreText, "Score", 660, 300);
}

void GameDesign::drawScore(RenderWindow& window, int score) {
    drawTextCentered(window, Score, to_string(score), 660, 350);
}

void GameDesign::drawLivesLogo(RenderWindow& window) {
    drawTextCentered(window, textLives, "Lives", 660, 200);
}

void GameDesign::drawHighScoreLogo(RenderWindow& window) {
    drawTextCentered(window, highScoreLogo, "High Score", 660, 400);
}

void GameDesign::drawHighScore(RenderWindow& window, int highscore) {
    drawTextCentered(window, highScore, to_string(highscore), 660, 450);
}

void GameDesign::drawLives(RenderWindow& window, int lives) {
    for (int i = 0; i < lives; ++i) {
        window.draw(livesSprite[i]);
    }
}

void GameDesign::Win(RenderWindow& window) {
    drawTextCentered(window, gameWinText, "You Win", 300, 300);
}

void GameDesign::drawStopOverlay(RenderWindow& window) {
    window.draw(overlaySprite);
    drawTextCentered(window, readyText, "Ready?", 260, 260);
}

void GameDesign::drawPausedScreen(RenderWindow& window) {
    window.draw(backgroundSprite);
    window.draw(overlaySprite);
    drawTextCentered(window, pausedText, "Paused", 260, 150);
    drawButton(window, resumeButtonSprite, resumeButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite, menuButtonSprite2, 260, 400);
}

void GameDesign::drawMenuScreen(RenderWindow& window) {
    window.draw(menuSprite);
    drawButton(window, gameLogoSprite, gameLogoSprite2, 400, 200);
    drawButton(window, startButtonSprite, startButtonSprite2, 400, 350);
    drawButton(window, quitButtonSprite, quitButtonSprite2, 400, 450);
}

void GameDesign::drawGameOverScreen(RenderWindow& window) {
    window.draw(backgroundSprite);
    window.draw(overlaySprite);
    drawTextCentered(window, gameOverText, "Game Over!", 260, 150);
    drawButton(window, playAgainButtonSprite, playAgainButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite, menuButtonSprite2, 260, 400);
}

void GameDesign::drawGameWinScreen(RenderWindow& window) {
    window.draw(backgroundSprite);
    window.draw(overlaySprite);
    drawTextCentered(window, gameWinText, "You Win!", 260, 150);
    drawButton(window, playAgainButtonSprite, playAgainButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite,menuButtonSprite2, 260, 400);
}

void GameDesign::drawButton(RenderWindow& window, Sprite& button, Sprite& hoverButton, float x, float y) {
    // Set the origin to the center of the sprite
    FloatRect bounds = button.getLocalBounds();
    button.setOrigin(bounds.width / 2, bounds.height / 2);
    hoverButton.setOrigin(bounds.width / 2, bounds.height / 2);

    // Update the position after setting the origin
    button.setPosition(x, y);
    hoverButton.setPosition(x, y);

    // Get the mouse position
    Vector2i mousePosition = Mouse::getPosition(window);

    // Check if the mouse is within the adjusted button bounds
    if (button.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))) {
        window.draw(hoverButton);
    }
    else {
        window.draw(button);
    }
}

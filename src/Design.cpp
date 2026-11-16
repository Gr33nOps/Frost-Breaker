#include "Design.hpp"
#include "Constants.hpp"
#include <cmath>

namespace {
constexpr unsigned HeadingSize = 60;
constexpr unsigned SecondarySize = 40;
const Color HeadingColor(237, 247, 247);
const Color SecondaryTextColor(189, 223, 229);
const Color HeadingOutlineColor(40, 73, 85);
}

GameDesign::GameDesign() {
    if (!font.loadFromFile("assets/fonts/Jersey10-Regular.ttf")) {
        cerr << "Error loading font!" << endl;
    }
    loadTexture(pixelFontTexture, "assets/images/PixelFont.png", "pixel font atlas");
    loadTexture(overlayTexture, "assets/images/overlay.png", "background image");
    loadTexture(menuTexture, "assets/images/menu background.png", "background image");
    loadTexture(gameLogoTexture, "assets/images/logo.png", "background image");
    loadTexture(gameLogoTexture2, "assets/images/logo2.png", "background image");
    loadTexture(startButtonTexture, "assets/images/start.png", "button image");
    loadTexture(quitButtonTexture, "assets/images/Quit.png", "button image");
    loadTexture(resumeButtonTexture, "assets/images/resume.png", "button image");
    loadTexture(playAgainButtonTexture, "assets/images/play again.png", "button image");
    loadTexture(menuButtonTexture, "assets/images/menu.png", "menu image");
    loadTexture(startButtonTexture2, "assets/images/start2.png", "button image");
    loadTexture(quitButtonTexture2, "assets/images/quit2.png", "button image");
    loadTexture(resumeButtonTexture2, "assets/images/resume2.png", "button image");
    loadTexture(playAgainButtonTexture2, "assets/images/play again2.png", "button image");
    loadTexture(menuButtonTexture2, "assets/images/menu2.png", "menu image");
    loadTexture(backgroundTexture, "assets/images/background.png", "background image");

    setSpriteTexture();
    loadLivesTextures();
    setupTextElements();
}

void GameDesign::loadTexture(Texture &texture, const string &filePath, const string &errorMessage) {
    if (!texture.loadFromFile(filePath)) {
        cerr << "Error loading " << errorMessage << " from: " << filePath << endl;
    }
    texture.setSmooth(false);
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
        string fileName = "assets/images/" + to_string(i + 1) + " lives.png";
        if (!livesTexture[i].loadFromFile(fileName)) {
            cerr << "Error loading " << i + 1 << " lives!" << endl;
        }
        livesSprite[i].setTexture(livesTexture[i]);
        const Image image = livesTexture[i].copyToImage();
        unsigned int visibleWidth = 0;
        for (unsigned int x = 0; x < image.getSize().x; ++x) {
            for (unsigned int y = 0; y < image.getSize().y; ++y) {
                if (image.getPixel(x, y).a != 0)
                    visibleWidth = x + 1;
            }
        }
        livesSprite[i].setTextureRect(IntRect(0, 0, int(visibleWidth), int(image.getSize().y)));
        livesSprite[i].setOrigin(visibleWidth / 2.0f, image.getSize().y / 2.0f);
        livesSprite[i].setPosition(660, 250);
    }
}

void GameDesign::setupTextElements() {
    setupText(Score, SecondarySize, Color::White);
    setupText(levelText, SecondarySize, Color::White);
    setupText(highScore, SecondarySize, Color::White);
    for (Text *title : {&levelLogo, &textLives, &scoreText, &highScoreLogo, &readyText, &pausedText,
                        &gameWinText, &gameOverText}) {
        setupText(*title, HeadingSize, HeadingColor);
        title->setOutlineColor(HeadingOutlineColor);
        title->setOutlineThickness(2);
    }
}

void GameDesign::setupText(Text &textElement, int size, Color color) {
    textElement.setFont(font);
    textElement.setCharacterSize(size);
    textElement.setFillColor(color);
}

void GameDesign::drawTextCentered(RenderWindow &window, Text &textElement, const string &text,
                                  float x, float y) {
    textElement.setString(text);
    const FloatRect bounds = textElement.getLocalBounds();
    textElement.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    textElement.setPosition(std::round(x), std::round(y));
    window.draw(textElement);
}

void GameDesign::drawBackground(RenderWindow &window) {
    window.draw(backgroundSprite);
}

void GameDesign::drawLevelLogo(RenderWindow &window) {
    drawTextCentered(window, levelLogo, "Level", 660, 100);
}

void GameDesign::drawCurrentLevel(RenderWindow &window, int levelNumber) {
    drawTextCentered(window, levelText, to_string(levelNumber), 660, 150);
}

void GameDesign::drawScoreLogo(RenderWindow &window) {
    drawTextCentered(window, scoreText, "Score", 660, 300);
}

void GameDesign::drawScore(RenderWindow &window, int currentScore) {
    drawTextCentered(window, Score, to_string(currentScore), 660, 350);
}

void GameDesign::drawLivesLogo(RenderWindow &window) {
    drawTextCentered(window, textLives, "Lives", 660, 200);
}

void GameDesign::drawHighScoreLogo(RenderWindow &window) {
    drawTextCentered(window, highScoreLogo, "High Score", 660, 400);
}

void GameDesign::drawHighScore(RenderWindow &window, int highscore) {
    drawTextCentered(window, highScore, to_string(highscore), 660, 450);
}

void GameDesign::drawLives(RenderWindow &window, int remainingLives) {
    if (remainingLives >= 1 && remainingLives <= 5)
        window.draw(livesSprite[remainingLives - 1]);
}

void GameDesign::Win(RenderWindow &window) {
    drawTextCentered(window, gameWinText, "You Win", 300, 300);
}

void GameDesign::drawLavaTime(RenderWindow &window, float seconds) {
    if (seconds <= 0)
        return;
    Text timer;
    setupText(timer, SecondarySize, Color(255, 179, 79));
    drawTextCentered(window, timer, "Lava " + std::to_string(int(std::ceil(seconds))) + "s", 660,
                     520);
}

void GameDesign::drawStopOverlay(RenderWindow &window) {
    window.draw(overlaySprite);
    drawTextCentered(window, readyText, "READY?", 260, 220);
    Text hint;
    setupText(hint, SecondarySize, SecondaryTextColor);
    drawTextCentered(window, hint, "MOUSE / ARROWS TO MOVE", 260, 325);
    drawTextCentered(window, hint, "CLICK / SPACE TO LAUNCH", 260, 370);
}

void GameDesign::drawPausedScreen(RenderWindow &window) {
    window.draw(overlaySprite);
    drawTextCentered(window, pausedText, "Paused", 260, 205);
    drawButton(window, resumeButtonSprite, resumeButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite, menuButtonSprite2, 260, 400);
}

void GameDesign::drawMenuScreen(RenderWindow &window) {
    window.draw(menuSprite);
    drawButton(window, gameLogoSprite, gameLogoSprite2, 403, 206);
    drawButton(window, startButtonSprite, startButtonSprite2, 400, 350);
    drawButton(window, quitButtonSprite, quitButtonSprite2, 400, 450);
}

void GameDesign::drawGameOverScreen(RenderWindow &window) {
    window.draw(overlaySprite);
    drawTextCentered(window, gameOverText, "Game Over!", 260, 205);
    drawButton(window, playAgainButtonSprite, playAgainButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite, menuButtonSprite2, 260, 400);
}

void GameDesign::drawGameWinScreen(RenderWindow &window) {
    window.draw(overlaySprite);
    drawTextCentered(window, gameWinText, "You Win!", 260, 205);
    Text ending;
    setupText(ending, SecondarySize, SecondaryTextColor);
    drawTextCentered(window, ending, "The valley is free!", 260, 245);
    drawButton(window, playAgainButtonSprite, playAgainButtonSprite2, 260, 300);
    drawButton(window, menuButtonSprite, menuButtonSprite2, 260, 400);
}

void GameDesign::drawButton(RenderWindow &window, Sprite &button, Sprite &hoverButton, float x,
                            float y) {
    FloatRect bounds = button.getLocalBounds();
    button.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    hoverButton.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);

    button.setPosition(x, y);
    hoverButton.setPosition(x, y);

    Vector2i mousePosition = Mouse::getPosition(window);

    if (button.getGlobalBounds().contains(static_cast<float>(mousePosition.x),
                                          static_cast<float>(mousePosition.y))) {
        window.draw(hoverButton);
    } else {
        window.draw(button);
    }
}

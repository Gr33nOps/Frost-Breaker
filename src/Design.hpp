#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

class GameDesign {
public:
    GameDesign();

    void drawBackground(RenderWindow& window);
    void drawLevelLogo(RenderWindow& window);
    void drawCurrentLevel(RenderWindow& window, int currentLevel);
    void drawScoreLogo(RenderWindow& window);
    void drawScore(RenderWindow& window, int score);
    void drawLivesLogo(RenderWindow& window);
    void drawHighScoreLogo(RenderWindow& window);
    void drawHighScore(RenderWindow& window, int highscore);
    void drawLives(RenderWindow& window, int lives);
    void Win(RenderWindow& window);
    void drawStopOverlay(RenderWindow& window);
    void drawPausedScreen(RenderWindow& window);
    void drawMenuScreen(RenderWindow& window);
    void drawGameOverScreen(RenderWindow& window);
    void drawGameWinScreen(RenderWindow& window);
    void drawHoverButton(RenderWindow& window, Sprite& buttonSprite, int x, int y);
    void handleHover(RenderWindow& window);

    Font font;
    Texture overlayTexture, menuTexture, gameLogoTexture, gameLogoTexture2;
    Texture startButtonTexture, quitButtonTexture, resumeButtonTexture, playAgainButtonTexture, menuButtonTexture, backgroundTexture;
    Texture startButtonTexture2, quitButtonTexture2, resumeButtonTexture2, playAgainButtonTexture2, menuButtonTexture2;
    Texture livesTexture[5];

    Sprite overlaySprite, menuSprite, gameLogoSprite, gameLogoSprite2;
    Sprite startButtonSprite, quitButtonSprite, resumeButtonSprite, playAgainButtonSprite, menuButtonSprite, backgroundSprite;
    Sprite startButtonSprite2, quitButtonSprite2, resumeButtonSprite2, playAgainButtonSprite2, menuButtonSprite2;
    Sprite livesSprite[5];

    Text Score, levelLogo, levelText, scoreText, textLives;
    Text gameOverText, gameWinText, highScoreLogo, highScore, readyText, pausedText;

    int currentLevel = 1;

    void loadTexture(Texture& texture, const string& filePath, const string& errorMessage);
    void setSpriteTexture();
    void loadLivesTextures();
    void setupTextElements();
    void setupText(Text& textElement, int size, Color color);
    void drawTextCentered(RenderWindow& window, Text& textElement, const string& text, float x, float y);
    void drawButton(RenderWindow& window, Sprite& button, Sprite& hoverButton, float x, float y);
};


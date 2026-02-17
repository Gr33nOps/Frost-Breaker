#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Logic.hpp"
#include "Constants.hpp"

class GameUi {
public:
    sf::RectangleShape boxShape;
    sf::Texture menuTexture;
    Sprite menuSprite;
    sf::Texture overlayTexture;
    Sprite overlaySprite;
    Texture backgroundTexture;
    Sprite backgroundSprite;
    Texture livesTexture[5];
    Sprite livesSprite[5];
    sf::Text levelLogo;
    sf::Text levelText;
    sf::Text scoreText;
    sf::Text Score;
    sf::Text gameOverText;
    sf::Text gameWinText;
    sf::Text textLives;
    sf::Font font;
    sf::Text highScoreLogo;
    sf::Text highScore;
    sf::Text readyText;
    sf::Text pausedText;
    sf::Texture gameLogoTexture;
    Sprite gameLogoSprite;
    sf::Texture startButtonTexture;
    Sprite startButtonSprite;
    sf::Texture quitButtonTexture;
    Sprite quitButtonSprite;

    sf::FloatRect startButtonBounds;
    sf::FloatRect quitButtonBounds;

    GameState gameState = STOP;

    GameUi();
    void menu(sf::RenderWindow& window, GameState& gameState);
    
};
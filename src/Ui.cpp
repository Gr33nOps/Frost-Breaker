#include "Logic.hpp"
#include "Design.hpp"
#include "Constants.hpp"

void GameUi::drawMenu(sf::RenderWindow& window,GameDesign& design, GameState& gameState) {
    sf::FloatRect gameLogobounds = design.gameLogoSprite.getLocalBounds();

    design.gameLogoSprite.setOrigin(gameLogobounds.width / 2, gameLogobounds.height / 2); // Set origin to the center
    design.gameLogoSprite.setPosition(400, 200); // Position the center at (600, 320)

    design.startButtonSprite.setPosition(280, 300); // Position the center at (600, 320)
    design.startButtonBounds = design.startButtonSprite.getGlobalBounds(); // Update bounds after changing position
    design.quitButtonSprite.setPosition(280, 400); // Position the center at (600, 320)
    design.quitButtonBounds = design.quitButtonSprite.getGlobalBounds();

    window.draw(design.menuSprite);
    window.draw(design.startButtonSprite);
    window.draw(design.quitButtonSprite);
    window.draw(design.gameLogoSprite);
}

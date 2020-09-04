#pragma once

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include <vector>

class Game
{
public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    static Game& getInstance();

    void processMessages();

    void handleEvent(const sf::Event& event);

    void update(const sf::Time& elapsedTime);

    void renderFrame(const sf::Time& elapsedTime);

    void blockingRun();

    void ParseEntityDescriptions(const std::vector<Config::Entity>& entityDescriptions);

    Scene scene;

private:
    Game();

    sf::RenderWindow window;
    sf::Vector2f cameraTranslation = { 0, 0 };

    sf::Clock clock;
    KeySet pressedKeys;
    KeySet releasedKeys;
    std::vector<sf::Shape*> toDrawList;
    std::vector<Task> toExecList;
    std::vector<sf::Sprite> sprites;
};
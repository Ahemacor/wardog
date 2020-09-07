#pragma once

#include "Scene.h"
#include "Config.h"
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

    Scene scene;

private:
    Game();

    sf::RenderWindow window;

    sf::Clock clock;
};
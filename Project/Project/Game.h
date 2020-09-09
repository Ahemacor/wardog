#pragma once

#include "Scene.h"
#include "Config.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>

struct Command
{
    enum class Type { LOAD, EXIT, MENU, BACK } type;
    std::vector<std::string> args;
};

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

    void exec(const std::string& action, const std::vector<std::string>& args);

    Scene scene;

    std::queue<Command> commands;

    sf::RenderWindow window;
private:
    Game();

    sf::Clock clock;
};
#include "Game.h"
#include "CommonDefinitions.h"
#include <optional>
#include <cmath>

Game& Game::getInstance()
{
    static Game instance;
    return instance;
}

Game::Game() : window(sf::VideoMode(WINDOW_CONFIG.w, WINDOW_CONFIG.h), WINDOW_CONFIG.name)
{
    window.setVerticalSyncEnabled(WINDOW_CONFIG.vSynch);
    window.setTitle(WINDOW_CONFIG.name);
    sf::Image& icon = IMAGE(WINDOW_CONFIG.icon);
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}

void Game::processMessages()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        handleEvent(event);
    }
}

void Game::handleEvent(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::KeyPressed:
    break;

    case sf::Event::KeyReleased:
    break;

    case sf::Event::Closed:
        window.close();
        break;

    default:
        break;
    }
    if (event.type == sf::Event::Closed)
        window.close();
}

void Game::update(const sf::Time& elapsedTime)
{
    scene.update(elapsedTime);

    UPDATE_UI();

    const auto& winCfg = WINDOW_CONFIG;
    std::string windowTitle;
    windowTitle += "[";
    windowTitle += std::to_string(winCfg.w);
    windowTitle += ":";
    windowTitle += std::to_string(winCfg.h);
    windowTitle += "] ";
    windowTitle += (winCfg.vSynch) ? " [v-Synch ON] " : " [v-Synch OFF] ";
    windowTitle += CONFIG.currentLevel;
    window.setTitle(windowTitle);
}

void Game::renderFrame(const sf::Time& elapsedTime)
{
    window.clear(sf::Color::Black);

    window.draw(scene);

    window.draw(UI_INSTANCE);

    window.display();

    COUNT_RENDER(elapsedTime);
}

void Game::blockingRun()
{
    sf::Time elapsedTime = clock.restart();

    while (window.isOpen())
    {
        elapsedTime = clock.restart();
        processMessages();
        update(elapsedTime);
        renderFrame(elapsedTime);
    }
}
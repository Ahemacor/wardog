#include "Game.h"
#include "CommonDefinitions.h"
#include <optional>
#include <cmath>

Game& Game::getInstance()
{
    static Game instance;
    return instance;
}

Game::Game() : window(sf::VideoMode(WINDOW_CONFIG.w, WINDOW_CONFIG.h), WINDOW_CONFIG.name, sf::Style::Titlebar | sf::Style::Close)
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
        if (event.key.code == sf::Keyboard::Key::Escape)
        {
            if (scene.menuStack.size() > 0)
            {
                exec("BACK", {});
            }
            else if (scene.allMenu.count("Game Settings"))
            {
                exec("MENU", { "Game Settings" });
            }
        }
    break;

    case sf::Event::Closed:
        window.close();
        break;

    default:
        break;
    }
    if (event.type == sf::Event::Closed)
        window.close();

    UI_INSTANCE.handleEvent(event);
}

void Game::update(const sf::Time& elapsedTime)
{
    scene.update(elapsedTime);

    UPDATE_UI(elapsedTime);

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
        while (!commands.empty())
        {
            const Command& command = commands.front();
            switch (command.type)
            {
            case Command::Type::LOAD:
                UI_INSTANCE.clearStaticText();
                scene.clear();
                CONFIG.loadLevel(command.args.front(), GAME_INSTANCE.scene);
                break;

            case Command::Type::BACK:
                if (scene.menuStack.top().name != "Main menu") scene.menuStack.pop();
                break;

            case Command::Type::EXIT:
                    scene.clear();
                    window.close();
                break;

            case Command::Type::MENU:
                scene.menuStack.push(scene.allMenu[command.args.front()]);
                break;

            default:
                break;
            }
            commands.pop();
        }

        elapsedTime = clock.restart();
        processMessages();
        update(elapsedTime);
        renderFrame(elapsedTime);
    }
}

void Game::exec(const std::string& action, const std::vector<std::string>& args)
{
    const std::string LEVEL = "LEVEL";
    const std::string SETTINGS = "SETTINGS";
    const std::string EXIT = "EXIT";
    const std::string MENU = "MENU";
    const std::string BACK = "BACK";

    if (action == EXIT)
    {
        commands.push({ Command::Type::EXIT, args });
    }
    else if (action == LEVEL)
    {
        commands.push({ Command::Type::LOAD, args });
    }
    else if (action == MENU)
    {
        commands.push({ Command::Type::MENU, args });
    }
    else if (action == BACK)
    {
        commands.push({ Command::Type::BACK, args });
    }
}
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
    {
        pressedKeys.insert(event.key.code);
    }
    break;

    case sf::Event::KeyReleased:
    {
        releasedKeys.insert(event.key.code);
        pressedKeys.erase(event.key.code);

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
}

void Game::update(const sf::Time& elapsedTime)
{
    const int32 velocityIterations = 50;
    const int32 positionIterations = 50;
    scene.world.Step(elapsedTime.asSeconds(), velocityIterations, positionIterations);

    for (const auto& keyCode : releasedKeys)
    {
        auto find = g_controls.find(keyCode);
        if (find != g_controls.cend())
        {
            for (auto& action : g_controls[keyCode]) action(false);
        }
    }

    for (const auto& keyCode : pressedKeys)
    {
        auto find = g_controls.find(keyCode);
        if (find != g_controls.cend())
        {
            for (auto& action : g_controls[keyCode]) action(true);
        }
    }

    for (auto& entity : scene.sceneGraph)
    {
        for (auto& component : entity.components)
        {
            switch (component.type)
            {
            case Entity_::Component::Type::BODY:
                {
                    b2Body* pBody = std::get<b2Body*>(component.var);
                    const b2Vec2 bodyPosition = pBody->GetPosition();
                    entity.transformation.setPosition({ (float)meterToPixel(bodyPosition.x), (float)meterToPixel(bodyPosition.y) });
                    entity.transformation.setRotation(radianToDegree(pBody->GetAngle()));
                    entity.velocity = pBody->GetLinearVelocity();
                }
                break;

            case Entity_::Component::Type::SHAPE:
                {
                    sf::RectangleShape& shape = std::get<sf::RectangleShape>(component.var);
                }
                break;

            case Entity_::Component::Type::SPRITE:
                {
                    sf::Sprite& sprite= std::get<sf::Sprite>(component.var);
                }
                break;

            case Entity_::Component::Type::ANIMATION:
                {
                    Animation& animation = std::get<Animation>(component.var);

                    const float bias = 0.01f;
                    const float velVectLen = entity.velocity.Length();

                    if (velVectLen > bias)
                    {
                        animation.setType(Animation::Type::WALK);
                        float absX = std::fabsf(entity.velocity.x);
                        float absY = std::fabsf(entity.velocity.y);
                        if (absX >= absY)
                        {
                            if (entity.velocity.x <= -bias)     animation.setDirection(Animation::Direction::LEFT);
                            else if (entity.velocity.x >= bias) animation.setDirection(Animation::Direction::RIGHT);
                        }
                        else
                        {
                            if (entity.velocity.y <= -bias)     animation.setDirection(Animation::Direction::UP);
                            else if (entity.velocity.y >= bias) animation.setDirection(Animation::Direction::DOWN);
                        }
                    }
                    else
                    {
                        animation.setType(Animation::Type::IDLE);
                    }

                    sf::Sprite sprite = animation.getSprite(elapsedTime);
                }
                break;
                case Entity_::Component::Type::CAMERA:
                {
                    cameraTransform = entity.transformation;
                    gameView = std::get<sf::View>(component.var);
                }
                break;
            }
        }
    }

    releasedKeys.clear();

    UPDATE_UI();

    if (!AudioSystem::getInstance().isMusicPlaying() && CONFIG.musicPlaylist.size() > 0)
    {
        const std::string currentMusicName = AudioSystem::getInstance().getCurrentMusic();
        const auto& playlilst = CONFIG.musicPlaylist;
        int musicNameIdx = 0;
        for (const auto& musName : playlilst)
        {
            ++musicNameIdx;
            if (musName == currentMusicName)
            {
                break;
            }
        }
        musicNameIdx = musicNameIdx % playlilst.size();
        const std::string& nextMusic = playlilst[musicNameIdx];
        PLAY_MUSIC(playlilst[musicNameIdx]);
        LOG_INFO(std::string("play: ") + nextMusic);
    }

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
    gameView.setViewport(gameViewport);
    window.setView(gameView);

    for (auto& entity : scene.sceneGraph)
    {
        for (auto& component : entity.components)
        {
            sf::RenderStates renderState;
            renderState.transform *= cameraTransform.getTransform().getInverse();
            renderState.transform *= entity.transformation.getTransform();

            switch (component.type)
            {
                renderState.transform *= component.transformation.getTransform();
                case Entity_::Component::Type::BODY:
                {
                    b2Body* pBody = std::get<b2Body*>(component.var);
                    break;
                }

                case Entity_::Component::Type::SHAPE:
                {
                    sf::RectangleShape& shape = std::get<sf::RectangleShape>(component.var);
                    window.draw(shape, renderState);
                    break;
                }

                case Entity_::Component::Type::SPRITE:
                {
                    sf::Sprite& sprite = std::get<sf::Sprite>(component.var);
                    window.draw(sprite, renderState);
                    break;
                }

                case Entity_::Component::Type::ANIMATION:
                {
                    Animation& animation = std::get<Animation>(component.var);
                    sf::Sprite sprite = animation.getSprite(elapsedTime);
                    window.draw(sprite, renderState);
                    break;
                }
            }
        }
    }

    window.setView(window.getDefaultView());

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
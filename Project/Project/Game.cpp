#include "Game.h"
#include "CommonDefinitions.h"
#include <optional>
#include <cmath>

constexpr float SCALE_FACTOR = 100.0f;
static constexpr float pixelToMeter(int pixels) { return pixels / SCALE_FACTOR; }
static constexpr int meterToPixel(float meters) { return static_cast<int>(meters * SCALE_FACTOR); }
static constexpr float degreeToRadian(float degrees) { return degrees / 180 * M_PI; }
static constexpr float radianToDegree(float radians) { return radians * 180 * M_PI; }

Game& Game::getInstance()
{
    static Game instance;
    return instance;
}

Game::Game() : window(sf::VideoMode(WINDOW_CONFIG.w, WINDOW_CONFIG.h), WINDOW_CONFIG.name)
{
    window.setVerticalSyncEnabled(WINDOW_CONFIG.vSynch);
    std::string windowTitle;
    windowTitle += "[";
    windowTitle += std::to_string(WINDOW_CONFIG.w);
    windowTitle += ":";
    windowTitle += std::to_string(WINDOW_CONFIG.h);
    windowTitle += "] ";
    windowTitle += (WINDOW_CONFIG.vSynch) ? " [v-Synch ON] " : " [v-Synch OFF] ";
    windowTitle += WINDOW_CONFIG.name;
    window.setTitle(windowTitle);
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

    toDrawList.clear();
    toExecList.clear();
    sprites.clear();

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

    for (auto& entity : scene.entities)
    {
        const int bodyIndex = entity.components[static_cast<int>(Entity::ComponentType::BODY)];
        const int shapeIndex = entity.components[static_cast<int>(Entity::ComponentType::SHAPE)];
        const int spriteIndex = entity.components[static_cast<int>(Entity::ComponentType::SPRITE)];
        const int animationIndex = entity.components[static_cast<int>(Entity::ComponentType::ANIMATION)];

        std::optional<sf::Vector2f> bodyPosition;
        std::optional<float> bodyAngle;
        std::optional<b2Vec2> bodyLinearVelocity;

        if (bodyIndex >= 0)
        {
            b2Body* pBody = scene.bodies[bodyIndex];
            if (entity.cameraFocus)
            {
                cameraTranslation = { (float)meterToPixel(pBody->GetPosition().x) - window.getSize().x / 2,
                                      (float)meterToPixel(pBody->GetPosition().y) - (window.getSize().y - 200) };
            }
            bodyPosition = { (float)meterToPixel(pBody->GetPosition().x), (float)meterToPixel(pBody->GetPosition().y) };
            bodyAngle = radianToDegree(pBody->GetAngle());
            bodyLinearVelocity = pBody->GetLinearVelocity();
        }

        if (shapeIndex >= 0)
        {
            sf::Shape* shape = &scene.shapes[shapeIndex];

            if (bodyIndex < 0)
            {
                bodyPosition = shape->getPosition();
                bodyAngle = shape->getRotation();
            }
            else
            {
                if (bodyPosition.has_value())shape->setPosition(bodyPosition.value().x, bodyPosition.value().y);
                if (bodyAngle.has_value())shape->setRotation(bodyAngle.value());
            }
            toDrawList.push_back(shape);
        }

        if (spriteIndex >= 0)
        {
            sf::Sprite& sprite = scene.sprites[spriteIndex];

            if (bodyIndex < 0)
            {
                bodyPosition = { sprite.getGlobalBounds().left, sprite.getGlobalBounds().top };
                //bodyAngle = sprite->getRotation();
            }
            else
            {
                if (bodyPosition.has_value())sprite.setPosition(bodyPosition.value().x, bodyPosition.value().y);
                if (bodyAngle.has_value())sprite.setRotation(bodyAngle.value());
            }
            sprites.push_back(sprite);

        }

        if (animationIndex >= 0)
        {

            Animation& animation = scene.animations[animationIndex];

            if (bodyLinearVelocity.has_value())
            {
                const float velVectLen = bodyLinearVelocity.value().Length();
                const float bias = 0.01f;

                if (velVectLen > bias)
                {
                    animation.setType(Animation::Type::WALK);
                    float absX = std::fabsf(bodyLinearVelocity.value().x);
                    float absY = std::fabsf(bodyLinearVelocity.value().y);
                    if (absX >= absY)
                    {
                        if (bodyLinearVelocity.value().x <= -bias)     animation.setDirection(Animation::Direction::LEFT);
                        else if (bodyLinearVelocity.value().x >= bias) animation.setDirection(Animation::Direction::RIGHT);
                    }
                    else
                    {
                        if (bodyLinearVelocity.value().y <= -bias)     animation.setDirection(Animation::Direction::UP);
                        else if (bodyLinearVelocity.value().y >= bias) animation.setDirection(Animation::Direction::DOWN);
                    }
                }
                else
                {
                    animation.setType(Animation::Type::IDLE);
                }
            }
            sf::Sprite sprite = animation.getSprite(elapsedTime);
            if (bodyPosition.has_value())sprite.setPosition(bodyPosition.value().x, bodyPosition.value().y);
            if (bodyAngle.has_value())sprite.setRotation(bodyAngle.value());
            sprites.push_back(sprite);
        }
    }

    for (auto& action : toExecList)
    {
        action();
    }

    releasedKeys.clear();

    UPDATE_UI();
}

void Game::renderFrame(const sf::Time& elapsedTime)
{
    window.clear(sf::Color::Black);

    for (auto* shape : toDrawList)
    {
        shape->move(-cameraTranslation);
        window.draw(*shape);
        shape->move(cameraTranslation);
    }

    for (auto& sprite : sprites)
    {
        sprite.move(-cameraTranslation);
        window.draw(sprite);
        sprite.move(cameraTranslation);
    }

    window.draw(UI_INSTANCE);

    window.display();

    COUNT_RENDER(elapsedTime);
}

void Game::blockingRun()
{
    PLAY_MUSIC(MUSIC_AMBIENT_NAME);

    sf::Time elapsedTime = clock.restart();

    while (window.isOpen())
    {
        elapsedTime = clock.restart();
        processMessages();
        update(elapsedTime);
        renderFrame(elapsedTime);
    }
}

void Game::ParseEntityDescriptions(const std::vector<Config::Entity>& entityDescriptions)
{
    for (const auto& entityDescription : entityDescriptions)
    {
        Entity& entity = scene.GetEntityRef(scene.CreateEntity());
        entity.name = entityDescription.name;

        for (const auto& componentDescription : entityDescription.components)
        {
            switch (componentDescription.type)
            {
            case Config::Entity::Component::Type::BODY:
            {
                const float width = pixelToMeter(componentDescription.width);
                const float height = pixelToMeter(componentDescription.height);
                const float x = pixelToMeter(componentDescription.x);
                const float y = pixelToMeter(componentDescription.y);
                entity.AddRectangleBody(width, height, x, y, componentDescription.bodyType);
            }
            break;
            case Config::Entity::Component::Type::SHAPE:
            {
                const float width = componentDescription.width;
                const float height = componentDescription.height;
                const float x = componentDescription.x;
                const float y = componentDescription.y;
                entity.AddRectangleShape(width, height, x, y);
                if (!componentDescription.texture.empty())
                {
                    entity.GetRectangleShape().setTexture(&TEXTURE(componentDescription.texture));
                }
            }
            break;
            case Config::Entity::Component::Type::SPRITE:
            {
                const float width = componentDescription.width;
                const float height = componentDescription.height;
                const float x = componentDescription.x;
                const float y = componentDescription.y;
                if (!componentDescription.texture.empty())
                {
                    entity.AddSprite(width, height, x, y, componentDescription.texture);
                }
                else
                {
                    entity.AddRectangleShape(width, height, x, y);
                }
            }
            break;
            case Config::Entity::Component::Type::ANIMATION:
            {
                if (!componentDescription.animation.empty())
                {
                    for (auto& animationSettings : SPRITE_SHEET_DESCR_LIST)
                    {
                        if (animationSettings.name == componentDescription.animation)
                        {
                            entity.components[static_cast<std::size_t>(Entity::ComponentType::ANIMATION)] = scene.animations.size();
                            scene.animations.push_back(Animation(animationSettings));
                            break;
                        }
                    }
                }
            }
            break;
            case Config::Entity::Component::Type::CAMERA:
                entity.cameraFocus = true;
                break;
            }
        }

        LOG_INFO(std::wstring(L"Создан объект: ") + std::wstring(entity.name.cbegin(), entity.name.cend()));
    }
}
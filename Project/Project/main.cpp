#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <tinyxml.h>
#include <tinystr.h>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <array>
#include <set>
#include <functional>
#define _USE_MATH_DEFINES
#include <cmath>
#include <box2d/box2d.h>
#include <iostream>
#include <optional>

constexpr float SCALE_FACTOR = 100.0f;
sf::Vector2f cameraTranslation = { 0, 0 };
inline constexpr float pixelToMeter(int pixels) { return pixels / SCALE_FACTOR; }
inline constexpr int meterToPixel(float meters) { return static_cast<int>(meters * SCALE_FACTOR); }
inline constexpr float degreeToRadian(float degrees) { return degrees / 180 * M_PI; }
inline constexpr float radianToDegree(float radians) { return radians * 180 * M_PI; }

class Config
{
public:
    struct Window
    {
        int w = 400;
        int h = 400;
        std::string name = "Untitled";
    };

    struct Animation
    {
        int x_offset = 4;
        int y_offset = 27; 
        int width = 64;
        int height = 64;
        int scale = 5;

        struct AnimationLine
        {
            int row_id = 0;
            int num_frames = 0;
            int ms_per_frame = 0;
        };

        AnimationLine idle;
        AnimationLine walk;
    };

    Config(const std::string& filepath)
    : doc(filepath.c_str())
    , hDoc(&doc)
    , pElem(nullptr)
    , hRoot(0)
    {
        doc.LoadFile();
        pElem = hDoc.FirstChildElement().Element();
        hRoot = TiXmlHandle(pElem);
    }

    Window getWindowSettings()
    {
        Window windowSettings;
        TiXmlElement* pWindowNode = hRoot.FirstChild("GameWindow").FirstChild().Element();
        windowSettings.name = pWindowNode->Attribute("name");
        pWindowNode->QueryIntAttribute("w", &windowSettings.w);
        pWindowNode->QueryIntAttribute("h", &windowSettings.h);
        return windowSettings;
    }

    Animation getAnimationSettings()
    {
        Animation animationSettings;
        TiXmlHandle animationNode = hRoot.FirstChild("Animation");

        TiXmlElement* common = animationNode.FirstChild("Common").Element();
        common->QueryIntAttribute("x_offset", &animationSettings.x_offset);
        common->QueryIntAttribute("y_offset", &animationSettings.y_offset);
        common->QueryIntAttribute("width", &animationSettings.width);
        common->QueryIntAttribute("height", &animationSettings.height);
        common->QueryIntAttribute("scale", &animationSettings.scale);

        TiXmlElement* idle = animationNode.FirstChild("Idle").Element();
        idle->QueryIntAttribute("row", &animationSettings.idle.row_id);
        idle->QueryIntAttribute("num_frames", &animationSettings.idle.num_frames);
        idle->QueryIntAttribute("frame_time_ms", &animationSettings.idle.ms_per_frame);

        TiXmlElement* walk = animationNode.FirstChild("Walk").Element();
        walk->QueryIntAttribute("row", &animationSettings.walk.row_id);
        walk->QueryIntAttribute("num_frames", &animationSettings.walk.num_frames);
        walk->QueryIntAttribute("frame_time_ms", &animationSettings.walk.ms_per_frame);

        return animationSettings;
    }

private:
    TiXmlDocument doc;
    TiXmlHandle hDoc;
    TiXmlElement* pElem;
    TiXmlHandle hRoot;
};

using KeySet = std::set<sf::Keyboard::Key>;
using Action = std::function<void(bool)>;
using Task = std::function<void()>;
using KeyBindings = std::map<sf::Keyboard::Key, Action>;

class Animation
{
public:
    enum class Type
    {
        IDLE,
        WALK
    };

    enum class Direction
    {
        LEFT,
        RIGHT
    };

    Animation(const Config::Animation& animationSettings)
    {
        settings = animationSettings;
        texture.loadFromFile("textures\\spritelist.png");
        sprite.setTexture(texture);
        sprite.setOrigin(settings.width / 2.0f, settings.height / 2.0f);
        sprite.setScale((float)settings.scale, (float)settings.scale);
    }

    sf::Sprite getSprite(Type type, Direction direction, sf::Time elapsedTime)
    {
        const int baseIdleRowId = settings.idle.row_id - 1;
        const int baseWalkRowId = settings.walk.row_id - 1;
        const int backwardDirectionOffset = 1;

        int row_id = 0;
        int column_id = 0;

        switch (type)
        {
        case Animation::Type::IDLE:
            if (direction == Direction::RIGHT) row_id = baseIdleRowId;
            else row_id = baseIdleRowId + backwardDirectionOffset;
            column_id = getColumnId(settings.idle.ms_per_frame, settings.idle.num_frames, elapsedTime);
            break;

        case Animation::Type::WALK:
            if (direction == Direction::RIGHT) row_id = baseWalkRowId;
            else row_id = baseWalkRowId + backwardDirectionOffset;
            column_id = getColumnId(settings.walk.ms_per_frame, settings.walk.num_frames, elapsedTime);
            break;

        default:
            break;
        }

        sf::IntRect newTectureRect(settings.x_offset + (column_id * settings.width), 
                                   settings.y_offset + (row_id * settings.height), 
                                   settings.width, settings.height);

        sprite.setTextureRect(newTectureRect);
        return sprite;
    }

    sf::Sprite getSprite(sf::Time elapsedTime)
    {
        return getSprite(type, direction, elapsedTime);
    }

    void setPosition(float xpos, float ypos)
    {
        sprite.setPosition(xpos, ypos);
    }

    void setType(Type type) { this->type = type; }

    void setDirection(Direction direction) { this->direction = direction; }


private:
    int getColumnId(int msPerFrame, int numOfFrames, sf::Time elapsedTime)
    {
        time += elapsedTime;
        sf::Time fullAnimDuration = sf::milliseconds(msPerFrame * numOfFrames);
        if (time > fullAnimDuration) time %= fullAnimDuration;
        return time.asMilliseconds() / msPerFrame;
    }

    Config::Animation settings;

    sf::Texture texture;
    sf::Sprite sprite;
    Type type = Type::IDLE;
    Direction direction = Direction::RIGHT;
    
    sf::Time time;
};

using Component = std::variant<sf::Shape*, KeyBindings*, Animation*, b2Body*, sf::Sprite>;

struct Entity
{
    enum class ComponentType
    {
        BINDINGS,
        BODY,
        SHAPE,
        BACKGROUND,
        ANIMATION
    };

    std::map<ComponentType, Component> components;
};

class AudioSystem
{
public:
    enum class SoundType
    {
        STEP,

        COUNT
    };

    AudioSystem()
    {
        soundBuffers[static_cast<std::size_t>(SoundType::STEP)].loadFromFile("audio\\steps\\sand1.wav");
    }

    void playSound(SoundType type)
    {
        for (auto& player : players)
        {
            if (player.getStatus() != sf::Sound::Status::Playing)
            {
                player.setBuffer(soundBuffers[static_cast<std::size_t>(type)]);
                player.play();
            }
        }
    }

private:
    std::array<sf::SoundBuffer, static_cast<std::size_t>(SoundType::COUNT)> soundBuffers;
    std::array<sf::Sound, 10> players;
};

class Game
{
public:
    struct Scene
    {
        std::vector<Entity> entities;
    };

    Game(const Config::Window& windowSettings)
    : world(b2Vec2(0.0f, 9.8f))
    , window(sf::VideoMode(windowSettings.w, windowSettings.h), windowSettings.name)
    {}

    void processMessages()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            handleEvent(event);
        }
    }

    void handleEvent(const sf::Event& event)
    {
        switch (event.type)
        {
        case sf::Event::KeyPressed:
            pressedKeys.insert(event.key.code);
            break;

        case sf::Event::KeyReleased:
            releasedKeys.insert(event.key.code);
            pressedKeys.erase(event.key.code);
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

    void update(const sf::Time& elapsedTime)
    {
        const int32 velocityIterations = 6;
        const int32 positionIterations = 2;
        world.Step(elapsedTime.asSeconds(), velocityIterations, positionIterations);

        toDrawList.clear();
        toExecList.clear();
        sprites.clear();

        for (auto& entity : scene.entities)
        {
            bool controlEntity = false;
            std::optional<sf::Vector2f> bodyPosition;
            std::optional<float> bodyAngle;
            std::optional<b2Vec2> bodyLinearVelocity;
            for (auto& [type, component] : entity.components)
            {
                switch (type)
                {
                case Entity::ComponentType::BINDINGS:
                {
                    controlEntity = true;
                    auto& binds = std::get<KeyBindings*>(component);
                    for (const auto& keyCode : releasedKeys)
                    {
                        auto findBind = binds->find(keyCode);
                        if (findBind != binds->cend())
                        {
                            auto& [bindKey, bindAction] = *findBind;
                            Task task = [findBind]() {findBind->second(false); };
                            toExecList.push_back(task);
                        }
                    }

                    for (const auto& keyCode : pressedKeys)
                    {
                        auto findBind = binds->find(keyCode);
                        if (findBind != binds->cend())
                        {
                            auto& [bindKey, bindAction] = *findBind;
                            Task task = [findBind]() {findBind->second(true); };
                            toExecList.push_back(task);
                        }
                    }
                }
                    break;

                case Entity::ComponentType::BACKGROUND:
                {
                    sf::Sprite sprite = std::get<sf::Sprite>(component);
                    sprites.insert(sprites.cbegin(), sprite);
                }
                break;

                case Entity::ComponentType::SHAPE:
                {
                    sf::Shape* shape = std::get<sf::Shape*>(component);
                    if (bodyPosition.has_value())shape->setPosition(bodyPosition.value().x, bodyPosition.value().y);
                    if (bodyAngle.has_value())shape->setRotation(bodyAngle.value());
                    toDrawList.push_back(shape);
                }
                    break;

                case Entity::ComponentType::ANIMATION:
                {
                    Animation* animation = std::get<Animation*>(component);

                    if (bodyLinearVelocity.has_value())
                    {
                        const float bias = 0.01f;
                        auto type = Animation::Type::WALK;
                        if (bodyLinearVelocity.value().x <= -bias) animation->setDirection(Animation::Direction::LEFT);
                        else if (bodyLinearVelocity.value().x >= bias) animation->setDirection(Animation::Direction::RIGHT);
                        else type = Animation::Type::IDLE;
                        animation->setType(type);
                        if (type == Animation::Type::WALK)
                        {
                            audioSystem.playSound(AudioSystem::SoundType::STEP);
                        }
                    }
                    sf::Sprite sprite = animation->getSprite(elapsedTime);
                    if (bodyPosition.has_value())sprite.setPosition(bodyPosition.value().x, bodyPosition.value().y);
                    if (bodyAngle.has_value())sprite.setRotation(bodyAngle.value());
                    sprites.push_back(sprite);
                }
                    break;

                case Entity::ComponentType::BODY:
                {
                    b2Body* pBody = std::get<b2Body*>(component);
                    if (controlEntity) cameraTranslation = { (float)meterToPixel(pBody->GetPosition().x) - window.getSize().x / 2,
                                                             (float)meterToPixel(pBody->GetPosition().y) - (window.getSize().y - 200)};
                    bodyPosition = { (float)meterToPixel(pBody->GetPosition().x), (float)meterToPixel(pBody->GetPosition().y) };
                    bodyPosition = bodyPosition.value() - cameraTranslation;
                    bodyAngle = radianToDegree(pBody->GetAngle());
                    bodyLinearVelocity = pBody->GetLinearVelocity();
                }
                    break;

                default:
                    break;
                }
            }
        }

        for (auto& action : toExecList)
        {
            action();
        }

        releasedKeys.clear();
    }

    void renderFrame()
    {
        window.clear(sf::Color::White);
        for (auto& sprite : sprites) window.draw(sprite);
        for (auto* shape : toDrawList) window.draw(*shape);
        window.display();
    }

    void blockingRun()
    {
        sf::Time elapsedTime = clock.restart();

        while (window.isOpen())
        {
            elapsedTime = clock.restart();
            processMessages();
            update(elapsedTime);
            renderFrame();
        }
    }

    std::size_t AddEntity(Entity&& entity)
    {
        scene.entities.push_back(std::move(entity));
        return scene.entities.size();
    }

    b2World world;

private:
    sf::RenderWindow window;
    AudioSystem audioSystem;
    Scene scene;
    sf::Clock clock;
    KeySet pressedKeys;
    KeySet releasedKeys;
    std::vector<sf::Shape*> toDrawList;
    std::vector<Task> toExecList;
    std::vector<sf::Sprite> sprites;
};

int main()
{
    const std::string windowConfigPath = "config\\settings.xml";
    Config config(windowConfigPath);
    Config::Window windowSettings = config.getWindowSettings();
    Game game(windowSettings);

    const float player_w = 1;
    const float player_h = 1;
    const float player_x = 3;
    const float player_y = 0;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(player_x, player_y);
    b2Body* body = game.world.CreateBody(&bodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(player_w/2, player_h/2);
    b2CircleShape circleShape;
    circleShape.m_radius = player_w / 2;

    b2FixtureDef fixtureDef;
    //fixtureDef.shape = &dynamicBox;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.8f;
    fixtureDef.restitution = 0.3f;

    body->CreateFixture(&fixtureDef);
    body->SetFixedRotation(true);

    Config::Animation animationSettings = config.getAnimationSettings();

    Animation playerAnimation(animationSettings);
    //playerAnimation.setPosition(windowSettings.w / 2.0f, windowSettings.h / 2.0f);
    const float bias = 0.01f;
    const float walkForce = 10.0f;
    const float jumpInpulse = 3.0f;
    const float maxVelocity = 5;

    KeyBindings playerControls;
    playerControls[sf::Keyboard::A] = [&](bool pressed)
    {
        auto linearVelocity = body->GetLinearVelocity();
        if (body->GetLinearVelocity().x >= -maxVelocity && body->GetLinearVelocity().x <= 0)
        {
            body->ApplyForceToCenter({ -walkForce, 0 }, true);
        }
    };

    playerControls[sf::Keyboard::D] = [&](bool pressed)
    {
        auto linearVelocity = body->GetLinearVelocity();
        if (body->GetLinearVelocity().x >= 0 && body->GetLinearVelocity().x <= maxVelocity)
        {
            body->ApplyForceToCenter({ walkForce, 0 }, true);
        }
    };

    playerControls[sf::Keyboard::Space] = [&](bool pressed)
    {
        auto linearVelocity = body->GetLinearVelocity();

        if (body->GetLinearVelocity().y <= bias && body->GetLinearVelocity().y >= -bias)
        {
            body->ApplyLinearImpulseToCenter({ 0, -jumpInpulse }, true);
        }
    };

    sf::RectangleShape playerRect;
    playerRect.setFillColor(sf::Color::Green);
    playerRect.setSize({ meterToPixel(player_w), meterToPixel(player_h) });
    playerRect.setOrigin(playerRect.getSize().x / 2, playerRect.getSize().y / 2);

    {
        Entity playerEntity;
        playerEntity.components[Entity::ComponentType::BINDINGS] = &playerControls;
        playerEntity.components[Entity::ComponentType::ANIMATION] = &playerAnimation;
        //playerEntity.components[Entity::ComponentType::SHAPE] = &playerRect;
        playerEntity.components[Entity::ComponentType::BODY] = body;

        game.AddEntity(std::move(playerEntity));
    }

    const float groundWidth = 10;
    const float groundHeight = 0.1;
    const float groundX = 5; //windowSettings.w / 2.0f;
    const float groundY = 5; //windowSettings.h - groundHeight;

    sf::RectangleShape groundRect;
    groundRect.setFillColor(sf::Color::Green);
    groundRect.setSize({ meterToPixel(groundWidth), meterToPixel(groundHeight) });
    groundRect.setOrigin(groundRect.getSize().x/ 2 , groundRect.getSize().y / 2);

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(groundX, groundY);
    b2Body* groundBody = game.world.CreateBody(&groundBodyDef);

    b2PolygonShape groundBox;
    groundBox.SetAsBox(groundWidth/2, groundHeight/2);

    b2FixtureDef groundfixtureDef;
    groundfixtureDef.shape = &groundBox;
    groundfixtureDef.density = 0.0f;
    groundfixtureDef.friction = 0.8f;
    groundBody->CreateFixture(&groundfixtureDef);

    {
        Entity groundEntity;
        groundEntity.components[Entity::ComponentType::SHAPE] = &groundRect;
        groundEntity.components[Entity::ComponentType::BODY] = groundBody;

        game.AddEntity(std::move(groundEntity));
    }

    const float rightWallWidth = 0.1;
    const float rightWallHeight = 10;
    const float rightWallX = 10;
    const float rightWallY = 0;

    sf::RectangleShape rightWallRect;
    rightWallRect.setFillColor(sf::Color::Green);
    rightWallRect.setSize({ meterToPixel(rightWallWidth), meterToPixel(rightWallHeight) });
    rightWallRect.setOrigin(rightWallRect.getSize().x / 2, rightWallRect.getSize().y / 2);

    b2BodyDef rightWallBodyDef;
    rightWallBodyDef.position.Set(rightWallX, rightWallY);
    b2Body* rightWallBody = game.world.CreateBody(&rightWallBodyDef);

    b2PolygonShape rightWallBox;
    rightWallBox.SetAsBox(rightWallWidth / 2, rightWallHeight / 2);

    b2FixtureDef rightWallfixtureDef;
    rightWallfixtureDef.shape = &rightWallBox;
    rightWallfixtureDef.density = 0.0f;
    rightWallfixtureDef.friction = 0.8f;
    rightWallBody->CreateFixture(&rightWallfixtureDef);

    {
        Entity rightWallEntity;
        rightWallEntity.components[Entity::ComponentType::SHAPE] = &rightWallRect;
        rightWallEntity.components[Entity::ComponentType::BODY] = rightWallBody;

        game.AddEntity(std::move(rightWallEntity));
    }

    const float leftWallWidth = 0.1;
    const float leftWallHeight = 10;
    const float leftWallX = 0;
    const float leftWallY = 0;

    sf::RectangleShape leftWallRect;
    leftWallRect.setFillColor(sf::Color::Green);
    leftWallRect.setSize({ meterToPixel(leftWallWidth), meterToPixel(leftWallHeight) });
    leftWallRect.setOrigin(leftWallRect.getSize().x / 2, leftWallRect.getSize().y / 2);

    b2BodyDef leftWallBodyDef;
    leftWallBodyDef.position.Set(leftWallX, leftWallY);
    b2Body* leftWallBody = game.world.CreateBody(&leftWallBodyDef);

    b2PolygonShape leftWallBox;
    leftWallBox.SetAsBox(leftWallWidth / 2, leftWallHeight / 2);

    b2FixtureDef leftWallfixtureDef;
    leftWallfixtureDef.shape = &leftWallBox;
    leftWallfixtureDef.density = 0.0f;
    leftWallfixtureDef.friction = 0.8f;
    leftWallBody->CreateFixture(&leftWallfixtureDef);

    {
        Entity leftWallEntity;
        leftWallEntity.components[Entity::ComponentType::SHAPE] = &leftWallRect;
        leftWallEntity.components[Entity::ComponentType::BODY] = leftWallBody;

        game.AddEntity(std::move(leftWallEntity));
    }

    const float box_w = 1;
    const float box_h = 0.5;
    const float box_x = 3;
    const float box_y = 3;

    b2BodyDef boxBodyDef;
    boxBodyDef.type = b2_dynamicBody;
    boxBodyDef.position.Set(box_x, box_y);
    b2Body* boxBody = game.world.CreateBody(&boxBodyDef);

    b2PolygonShape boxDynamicBox;
    boxDynamicBox.SetAsBox(box_w / 2, box_h / 2);


    b2FixtureDef boxFixtureDef;
    boxFixtureDef.shape = &boxDynamicBox;
    boxFixtureDef.density = 10.0f;
    boxFixtureDef.friction = 0.8f;
    boxFixtureDef.restitution = 0.3f;

    boxBody->CreateFixture(&boxFixtureDef);

    sf::RectangleShape boxRect;
    boxRect.setFillColor(sf::Color::Red);
    boxRect.setSize({ meterToPixel(box_w), meterToPixel(box_h) });
    boxRect.setOrigin(boxRect.getSize().x / 2, boxRect.getSize().y / 2);

    {
        Entity boxEntity;
        boxEntity.components[Entity::ComponentType::SHAPE] = &boxRect;
        boxEntity.components[Entity::ComponentType::BODY] = boxBody;

        game.AddEntity(std::move(boxEntity));
    }

    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("textures\\background.png");
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    {
        Entity backgroundEntity;
        backgroundEntity.components[Entity::ComponentType::BACKGROUND] = backgroundSprite;

        game.AddEntity(std::move(backgroundEntity));
    }

    sf::Music music;
    if (!music.openFromFile("audio\\ambient\\ambient.wav")) return -1;

    music.setLoop(true);
    music.setVolume(10.0f);
    music.play();

    game.blockingRun();

    return 0;
}
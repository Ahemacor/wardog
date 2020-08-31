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

enum class AnimationType
{
    IDLE,
    WALK
};

static constexpr const char* AnimationNames[] =
{
    "Idle",
    "Walk"
};

enum class DirectionType
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

static constexpr const char* DirectionNames[] =
{
    "Up",
    "Down",
    "Left",
    "Right"
};

struct Resource
{
    enum class Type
    {
        TEXTURE,
        SOUND,
        MUSIC
    };

    void load(Type resoureType, const std::string& resourceName, const std::string& path)
    {
        type = resoureType;
        name = resourceName;

        switch (type)
        {
        case Resource::Type::TEXTURE:
            value = sf::Texture();
            std::get<sf::Texture>(value).loadFromFile(path);
            break;
        case Resource::Type::SOUND:
            value = sf::SoundBuffer();
            std::get<sf::SoundBuffer>(value).loadFromFile(path);
            break;
        case Resource::Type::MUSIC:
            value = new sf::Music;
            std::get<sf::Music*>(value)->openFromFile(path);
            break;
        default:
            break;
        }
    }

    Type getType() { return type; }

    std::string getName() { return name; }

    sf::Texture& getTextureRef()
    {
        return std::get<sf::Texture>(value);
    }

    sf::SoundBuffer& getSoundRef()
    {
        return std::get<sf::SoundBuffer>(value);
    }

    sf::Music& getMusicRef()
    {
        return *std::get<sf::Music*>(value);
    }

private:
    Type type;
    std::string name;
    std::variant<sf::Texture, sf::SoundBuffer, sf::Music*> value;
};


const std::string TEXTURE_SPRITE_NAME = "spritelist";
const std::string TEXTURE_TEST_NAME = "test";
const std::string SOUND_STEP_NAME = "step";
const std::string MUSIC_AMBIENT_NAME = "ambient";
std::unordered_map<std::string, Resource> g_resources;

class Config
{
public:
    static constexpr const char* PATH_DELIMITER = "\\";

    static constexpr const char* XML_TAG_SPRITE_SHEET = "Spritesheet";
    static constexpr const char* XML_TAG_SPRITE_SHEET_TEXTURE = "texture";
    static constexpr const char* XML_TAG_SPRITE_SHEET_X_OFFSET = "x_offset";
    static constexpr const char* XML_TAG_SPRITE_SHEET_Y_OFFSET = "y_offset";
    static constexpr const char* XML_TAG_SPRITE_SHEET_WIDTH = "width";
    static constexpr const char* XML_TAG_SPRITE_SHEET_HEIGHT = "height";
    static constexpr const char* XML_TAG_SPRITE_SHEET_SCALE = "scale";

    static constexpr const char* XML_TAG_SPRITE_SHEET_ANIMATION = "Animation";
    static constexpr const char* XML_TAG_SPRITE_SHEET_ANIMATION_NAME = "name";
    static constexpr const char* XML_TAG_SPRITE_SHEET_ANIMATION_FRAMES = "num_frames";
    static constexpr const char* XML_TAG_SPRITE_SHEET_ANIMATION_TIME = "frame_time_ms";

    static constexpr const char* XML_TAG_SPRITE_SHEET_ANIMATION_ROW_INDEX = "row_index";

    struct Window
    {
        int w = 400;
        int h = 400;
        std::string name = "Untitled";
    };

    struct Spritesheet
    {
        std::string texture; 
        int x_offset = 0;
        int y_offset = 0;
        int width = 0;
        int height = 0;
        int scale = 0;

        struct Animation
        {
            std::string name;
            int num_frames = 0;
            int ms_per_frame = 0;
            std::map<std::string, int> row_index;
        };

        std::map<std::string, Animation> animations;
    };

    struct Entity
    {
        std::string name;

        struct Component
        {
            enum class Type
            {
                BODY,
                SHAPE
            };
            Type type;

            b2BodyType bodyType;
            std::string texture;
            float width;
            float height;
            float x;
            float y;
        };
        std::vector<Component> components;
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
        LoadResoures();
    }

    void LoadResoures()
    {
        static constexpr const char* XML_TAG_RESOURCES = "Resources";
        static constexpr const char* XML_TAG_RESOURCES_TYPE = "type";
        static constexpr const char* XML_TAG_RESOURCES_DIRECTORY = "directory";
        static constexpr const char* XML_TAG_RESOURCE_TYPE_TEXTURE = "Texture";
        static constexpr const char* XML_TAG_RESOURCE_TYPE_SOUND = "Sound";
        static constexpr const char* XML_TAG_RESOURCE_TYPE_MUSIC = "Music";
        static constexpr const char* XML_TAG_RESOURCE = "Resource";
        static constexpr const char* XML_TAG_RESOURCE_NAME = "name";
        static constexpr const char* XML_TAG_RESOURCE_EXT = "ext";

        TiXmlElement* resouresElem = hRoot.FirstChild(XML_TAG_RESOURCES).Element();
        for (resouresElem; resouresElem != nullptr; resouresElem = resouresElem->NextSiblingElement())
        {
            const std::string elemName = resouresElem->Value();
            if (elemName != XML_TAG_RESOURCES) continue;

            const std::string typeString = resouresElem->Attribute(XML_TAG_RESOURCES_TYPE);
            const std::string directory  = resouresElem->Attribute(XML_TAG_RESOURCES_DIRECTORY);

            Resource::Type type = Resource::Type::TEXTURE;
            if      (typeString == XML_TAG_RESOURCE_TYPE_TEXTURE) type = Resource::Type::TEXTURE;
            else if (typeString == XML_TAG_RESOURCE_TYPE_SOUND)   type = Resource::Type::SOUND;
            else if (typeString == XML_TAG_RESOURCE_TYPE_MUSIC)   type = Resource::Type::MUSIC;
            else continue; //ERROR 

            TiXmlElement* resourceElem = resouresElem->FirstChild(XML_TAG_RESOURCE)->ToElement();
            for (resourceElem; resourceElem != nullptr; resourceElem = resourceElem->NextSiblingElement())
            {
                const std::string resourceName = resourceElem->Attribute(XML_TAG_RESOURCE_NAME);
                const std::string resourceExt = resourceElem->Attribute(XML_TAG_RESOURCE_EXT);
                const std::string resourcePath = directory + PATH_DELIMITER + resourceName + "." + resourceExt;

                g_resources[resourceName].load(type, resourceName, resourcePath);
            }
        }
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

    std::vector<Spritesheet> getAnimationSettings()
    {
        std::vector<Spritesheet> spriteSheets;

        TiXmlElement* spriteSheetElem = hRoot.FirstChild(Config::XML_TAG_SPRITE_SHEET).Element();
        for (spriteSheetElem; spriteSheetElem != nullptr; spriteSheetElem = spriteSheetElem->NextSiblingElement())
        {
            const std::string elemName = spriteSheetElem->Value();
            if (elemName != XML_TAG_SPRITE_SHEET) continue;

            Config::Spritesheet spriteSheetInfo;
            spriteSheetInfo.texture = spriteSheetElem->Attribute(Config::XML_TAG_SPRITE_SHEET_TEXTURE);
            spriteSheetElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_X_OFFSET, &spriteSheetInfo.x_offset);
            spriteSheetElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_Y_OFFSET, &spriteSheetInfo.y_offset);
            spriteSheetElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_WIDTH, &spriteSheetInfo.width);
            spriteSheetElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_HEIGHT, &spriteSheetInfo.height);
            spriteSheetElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_SCALE, &spriteSheetInfo.scale);

            TiXmlElement* animationElem = spriteSheetElem->FirstChild(Config::XML_TAG_SPRITE_SHEET_ANIMATION)->ToElement();
            for (animationElem; animationElem != nullptr; animationElem = animationElem->NextSiblingElement())
            {
                Config::Spritesheet::Animation animationInfo;
                animationInfo.name = animationElem->Attribute(Config::XML_TAG_SPRITE_SHEET_ANIMATION_NAME);
                animationElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_ANIMATION_FRAMES, &animationInfo.num_frames);
                animationElem->QueryIntAttribute(Config::XML_TAG_SPRITE_SHEET_ANIMATION_TIME, &animationInfo.ms_per_frame);
                for (TiXmlElement* animationChildElem = animationElem->FirstChildElement() ; animationChildElem != nullptr; animationChildElem = animationChildElem->NextSiblingElement())
                {
                    const char* directionName = animationChildElem->Value();
                    const int row_index = animationChildElem->FirstAttribute()->IntValue();
                    animationInfo.row_index[directionName] = row_index;
                }
                spriteSheetInfo.animations[animationInfo.name] = animationInfo;
            }

            spriteSheets.push_back(std::move(spriteSheetInfo));
        }

        return spriteSheets;
    }

    std::vector<Entity> getEntityDescriptions()
    {
        static constexpr const char* XML_TAG_ENTITY = "Entity";
        static constexpr const char* XML_TAG_ENTITY_NAME = "name";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_BODY = "Body";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_SHAPE = "Shape";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE = "type";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_TEXTURE = "texture";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE_STATIC = "static";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE_DYNAMIC = "dynamic";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_WIDTH = "width";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_HEIGHT = "height";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_X = "x";
        static constexpr const char* XML_TAG_ENTITY_COMPONENT_Y = "y";

        std::vector<Entity> entities;
        TiXmlElement* entityElem = hRoot.FirstChild(XML_TAG_ENTITY).Element();
        for (entityElem; entityElem != nullptr; entityElem = entityElem->NextSiblingElement())
        {
            const std::string elemName = entityElem->Value();
            if (elemName != XML_TAG_ENTITY) continue;

            Entity entityInfo;
            entityInfo.name = entityElem->Attribute(XML_TAG_ENTITY_NAME);

            TiXmlElement* componentElem = entityElem->FirstChild()->ToElement();
            for (componentElem; componentElem != nullptr; componentElem = componentElem->NextSiblingElement())
            {
                Entity::Component componentInfo;
                std::string componentName = componentElem->Value();
                if (componentName == XML_TAG_ENTITY_COMPONENT_BODY)
                {
                    componentInfo.type = Entity::Component::Type::BODY;
                    std::string componentTypeString = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TYPE);
                    if (componentTypeString == XML_TAG_ENTITY_COMPONENT_TYPE_STATIC)
                    {
                        componentInfo.bodyType = b2_staticBody;
                    }
                    else if (componentTypeString == XML_TAG_ENTITY_COMPONENT_TYPE_DYNAMIC)
                    {
                        componentInfo.bodyType = b2_dynamicBody;
                    }
                }
                else if (componentName == XML_TAG_ENTITY_COMPONENT_SHAPE)
                {
                    componentInfo.type = Entity::Component::Type::SHAPE;
                }

                const char* pTexture = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TEXTURE);
                if (pTexture != nullptr)
                {
                    componentInfo.texture = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TEXTURE);
                }

                componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_WIDTH, &componentInfo.width);
                componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_HEIGHT, &componentInfo.height);
                componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_X, &componentInfo.x);
                componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_Y, &componentInfo.y);

                entityInfo.components.push_back(componentInfo);
            }

            entities.push_back(entityInfo);
        }

        return entities;
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
    Animation(const Config::Spritesheet& animationSettings)
    {
        settings = animationSettings;
        sprite.setOrigin(settings.width / 2.0f, settings.height / 2.0f);
        sprite.setScale((float)settings.scale, (float)settings.scale);
        sprite.setTexture(g_resources[settings.texture].getTextureRef());
    }

    sf::Sprite getSprite(AnimationType type, DirectionType direction, sf::Time elapsedTime)
    {
        const char* directionName = DirectionNames[static_cast<int>(direction)];
        const char* animationName = AnimationNames[static_cast<int>(type)];

        int row_id = 0;
        int column_id = 0;

        auto& animationInfo = settings.animations[animationName];
        row_id = animationInfo.row_index[directionName];
        column_id = getColumnId(animationInfo.ms_per_frame, animationInfo.num_frames, elapsedTime);

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

    void setType(AnimationType type) { this->type = type; }
    AnimationType getType() { return type; }

    void setDirection(DirectionType direction) { this->direction = direction; }
    DirectionType getDirection() { direction; }


private:
    int getColumnId(int msPerFrame, int numOfFrames, sf::Time elapsedTime)
    {
        time += elapsedTime;
        sf::Time fullAnimDuration = sf::milliseconds(msPerFrame * numOfFrames);
        if (time > fullAnimDuration) time %= fullAnimDuration;
        return time.asMilliseconds() / msPerFrame;
    }

    Config::Spritesheet settings;

    sf::Sprite sprite;
    AnimationType type = AnimationType::IDLE;
    DirectionType direction = DirectionType::RIGHT;
    
    sf::Time time;
};

class AudioSystem
{
public:
    enum class SoundType
    {
        STEP
    };

    void playSound(SoundType type)
    {
        for (auto& player : players)
        {
            if (player.getStatus() != sf::Sound::Status::Playing)
            {
                switch (type)
                {
                case SoundType::STEP:
                    player.setBuffer(g_resources[SOUND_STEP_NAME].getSoundRef());
                    break;
                }

                player.play();
            }
        }
    }

    void playMusic(const std::string& musicName, float volume = 10, bool loop = true)
    {
        if (!currentMusicName.empty())
        {
            sf::Music& music = g_resources[currentMusicName].getMusicRef();
            music.stop();
        }

        currentMusicName = musicName;
        sf::Music& music = g_resources[currentMusicName].getMusicRef();
        music.setLoop(true);
        music.setVolume(10.0f);
        music.play();
    }

private:
    std::array<sf::Sound, 10> players;
    std::string currentMusicName;
};

struct Scene;

struct Entity
{
    enum class ComponentType
    {
        BINDINGS,
        BODY,
        SHAPE,
        ANIMATION,

        COUNT
    };

    int GetComponentIndex(ComponentType type)
    {
        return components[static_cast<std::size_t>(type)];
    }

    void SetComonentIndex(ComponentType type, int index)
    {
        components[static_cast<std::size_t>(type)] = index;
    }

    Entity(Scene& scene) : parentScene(scene) 
    {
        for (auto& component : components) component = -1;
    }

    void AddBinding(sf::Keyboard::Key key, Action action);
    KeyBindings& GetBindings();

    void AddRectangleShape(float width, float height, float xPos = 0, float yPos = 0);
    sf::RectangleShape& GetRectangleShape();

    void AddRectangleBody(float width, float height, float xPos = 0, float yPos = 0, b2BodyType type = b2_staticBody);
    b2Body& GetRectangleBody();

    std::array <int, static_cast<std::size_t>(ComponentType::COUNT)> components;
    Scene& parentScene;
    bool cameraFocus = false;
    std::string name;
};

struct Scene
{
    Scene() : world(b2Vec2(0.0f, 0.0f)) 
    {}

    std::size_t AddEntity(Entity&& entity)
    {
        const std::size_t entityId = entities.size();
        entities.push_back(std::move(entity));
        return entityId;
    }

    std::size_t CreateEntity()
    {
        const std::size_t entityId = entities.size();
        entities.push_back(Entity(*this));
        return entityId;
    }

    Entity& GetEntityRef(std::size_t entityId)
    {
        return entities[entityId];
    }

    std::vector<Entity> entities;
    std::vector<b2Body*> bodies;
    std::vector<sf::RectangleShape> shapes;
    std::vector<KeyBindings> bindings;
    std::vector<Animation> animations;
    std::vector<sf::Texture> textures;

    b2World world;
};

void Entity::AddBinding(sf::Keyboard::Key key, Action action)
{
    const int bindingIndex = GetComponentIndex(ComponentType::BINDINGS);
    if (bindingIndex < 0)
    {
        KeyBindings newBindings;
        newBindings[key] = action;
        SetComonentIndex(ComponentType::BINDINGS, parentScene.bindings.size());
        parentScene.bindings.push_back(newBindings);
    }
    else
    {
        KeyBindings& bindgings = parentScene.bindings[bindingIndex];
        bindgings[key] = action;
    }
}

KeyBindings& Entity::GetBindings()
{
    const int bindingIndex = GetComponentIndex(ComponentType::BINDINGS);
    return parentScene.bindings[bindingIndex];
}

void Entity::AddRectangleShape(float width, float height, float xPos, float yPos)
{
    sf::RectangleShape newRect;
    newRect.setSize({ width, height });
    newRect.setOrigin(width / 2, height / 2);
    newRect.setPosition(xPos, yPos);

    const int bindingIndex = GetComponentIndex(ComponentType::SHAPE);
    if (bindingIndex < 0)
    {
        SetComonentIndex(ComponentType::SHAPE, parentScene.shapes.size());
        parentScene.shapes.push_back(std::move(newRect));
    }
    else
    {
        parentScene.shapes[bindingIndex] = newRect;
    }
}

sf::RectangleShape& Entity::GetRectangleShape()
{
    const int bindingIndex = GetComponentIndex(ComponentType::SHAPE);
    return parentScene.shapes[bindingIndex];
}

void Entity::AddRectangleBody(float width, float height, float xPos, float yPos, b2BodyType type)
{
    b2BodyDef bodyDef;
    bodyDef.type = type;
    bodyDef.position.Set(xPos, yPos);
    bodyDef.angularDamping = 15.0f;
    bodyDef.linearDamping = 10.0f;
    b2Body* body = parentScene.world.CreateBody(&bodyDef);

    b2PolygonShape boxShape;
    boxShape.SetAsBox(width / 2, height / 2);

    b2FixtureDef fixture;
    fixture.shape = &boxShape;
    fixture.density = 1.0f;
    fixture.friction = 0.8f;
    body->CreateFixture(&fixture);

    const int bindingIndex = GetComponentIndex(ComponentType::BODY);
    if (bindingIndex < 0)
    {
        SetComonentIndex(ComponentType::BODY, parentScene.bodies.size());
        parentScene.bodies.push_back(body);
    }
    else
    {
        parentScene.bodies[bindingIndex] = body;
    }
}

b2Body& Entity::GetRectangleBody()
{
    const int bindingIndex = GetComponentIndex(ComponentType::BODY);
    return *parentScene.bodies[bindingIndex];
}

class Game
{
public:
    Game(const Config::Window& windowSettings)
    : window(sf::VideoMode(windowSettings.w, windowSettings.h), windowSettings.name)
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
        const int32 velocityIterations = 50;
        const int32 positionIterations = 50;
        scene.world.Step(elapsedTime.asSeconds(), velocityIterations, positionIterations);

        toDrawList.clear();
        toExecList.clear();
        sprites.clear();

        for (auto& entity : scene.entities)
        {
            const int bindingIndex = entity.components[static_cast<int>(Entity::ComponentType::BINDINGS)];
            const int bodyIndex = entity.components[static_cast<int>(Entity::ComponentType::BODY)];
            const int shapeIndex = entity.components[static_cast<int>(Entity::ComponentType::SHAPE)];
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
                //bodyPosition = bodyPosition.value() - cameraTranslation;
                bodyAngle = radianToDegree(pBody->GetAngle());
                bodyLinearVelocity = pBody->GetLinearVelocity();
            }

            if(bindingIndex >= 0)
            {
                auto& binds = scene.bindings[bindingIndex];
                for (const auto& keyCode : releasedKeys)
                {
                    auto findBind = binds.find(keyCode);
                    if (findBind != binds.cend())
                    {
                        auto& [bindKey, bindAction] = *findBind;
                        Task task = [findBind]() {findBind->second(false); };
                        toExecList.push_back(task);
                    }
                }

                for (const auto& keyCode : pressedKeys)
                {
                    auto findBind = binds.find(keyCode);
                    if (findBind != binds.cend())
                    {
                        auto& [bindKey, bindAction] = *findBind;
                        Task task = [findBind]() {findBind->second(true); };
                        toExecList.push_back(task);
                    }
                }
            }

            if(shapeIndex >= 0)
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

            if(animationIndex >= 0)
            {

                Animation& animation = scene.animations[animationIndex];

                if (bodyLinearVelocity.has_value())
                {
                    const float velVectLen = bodyLinearVelocity.value().Length();
                    const float bias = 0.01f;

                    if (velVectLen > bias)
                    {
                        animation.setType(AnimationType::WALK);
                        float absX = std::fabsf(bodyLinearVelocity.value().x);
                        float absY = std::fabsf(bodyLinearVelocity.value().y);
                        if (absX >= absY)
                        {
                            if (bodyLinearVelocity.value().x <= -bias)     animation.setDirection(DirectionType::LEFT);
                            else if (bodyLinearVelocity.value().x >= bias) animation.setDirection(DirectionType::RIGHT);
                        }
                        else
                        {
                            if (bodyLinearVelocity.value().y <= -bias)     animation.setDirection(DirectionType::UP);
                            else if (bodyLinearVelocity.value().y >= bias) animation.setDirection(DirectionType::DOWN);
                        }
                    }
                    else
                    {
                        animation.setType(AnimationType::IDLE);
                    }

                    // NEED TO MOVE TO SOMEWHERE !!!
                    if (animation.getType() == AnimationType::WALK)
                    {
                        audioSystem.playSound(AudioSystem::SoundType::STEP);
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
    }

    void renderFrame()
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

        window.display();
    }

    void blockingRun()
    {
        audioSystem.playMusic(MUSIC_AMBIENT_NAME);

        sf::Time elapsedTime = clock.restart();

        while (window.isOpen())
        {
            elapsedTime = clock.restart();
            processMessages();
            update(elapsedTime);
            renderFrame();
        }
    }

    void ParseEntityDescriptions(const std::vector<Config::Entity>& entityDescriptions)
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
                        entity.GetRectangleShape().setTexture(&g_resources[componentDescription.texture].getTextureRef());
                    }
                }
                break;
                }
            }
        }
    }

    Scene scene;

private:
    sf::RenderWindow window;
    AudioSystem audioSystem;
    
    sf::Clock clock;
    KeySet pressedKeys;
    KeySet releasedKeys;
    std::vector<sf::Shape*> toDrawList;
    std::vector<Task> toExecList;
    std::vector<sf::Sprite> sprites;
};

const float bias = 0.01f;
const float walkForce = 12.0f;
const float jumpInpulse = 3.0f;
const float maxVelocity = 5;

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

    Config::Spritesheet animationSettings = config.getAnimationSettings()[0];

    Animation playerAnimation(animationSettings);

    {
        const std::size_t newEntityId = game.scene.CreateEntity();
        Entity& entity = game.scene.GetEntityRef(newEntityId);

        entity.AddRectangleBody(player_w, player_h, player_x, player_y, b2_dynamicBody);
        b2Body* body = &entity.GetRectangleBody();
        body->SetFixedRotation(true);

        entity.AddBinding(sf::Keyboard::A, [body](bool pressed)
        {
            auto linearVelocity =  body->GetLinearVelocity();
            body->ApplyForceToCenter({ -walkForce, 0 }, true);
        });

        entity.AddBinding(sf::Keyboard::D, [body](bool pressed)
        {
            auto linearVelocity = body->GetLinearVelocity();
            body->ApplyForceToCenter({ walkForce, 0 }, true);
        });

        entity.AddBinding(sf::Keyboard::W, [&body](bool pressed)
        {
            auto linearVelocity = body->GetLinearVelocity();
            body->ApplyForceToCenter({ 0, -walkForce }, true);
        });

        entity.AddBinding(sf::Keyboard::S, [&body](bool pressed)
        {
            auto linearVelocity = body->GetLinearVelocity();
            body->ApplyForceToCenter({0, walkForce}, true);
        });

        entity.components[static_cast<std::size_t>(Entity::ComponentType::ANIMATION)] = game.scene.animations.size();
        game.scene.animations.push_back(playerAnimation);

        entity.cameraFocus = true;
    }

    std::vector<Config::Entity> entityDescriptions = config.getEntityDescriptions();
    game.ParseEntityDescriptions(entityDescriptions);

    game.blockingRun();

    return 0;
}
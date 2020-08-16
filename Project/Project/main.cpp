#include <SFML/Graphics.hpp>
#include <tinyxml.h>
#include <tinystr.h>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <set>
#include <functional>
#include <cmath>

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

using Component = std::variant<sf::Drawable*, KeyBindings*, Animation*>;

struct Entity
{
    enum class ComponentType
    {
        BINDINGS,
        DRAWABLE,
        ANIMATION
    };

    std::map<ComponentType, Component> components;
};

class Game
{
public:
    struct Scene
    {
        std::vector<Entity> entities;
    };

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
        toDrawList.clear();
        toExecList.clear();
        sprites.clear();

        for (auto& entity : scene.entities)
        {
            for (auto& [type, component] : entity.components)
            {
                switch (type)
                {
                case Entity::ComponentType::BINDINGS:
                {
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

                case Entity::ComponentType::DRAWABLE:
                    toDrawList.push_back(std::get<sf::Drawable*>(component));
                    break;

                case Entity::ComponentType::ANIMATION:
                    sprites.push_back(std::get<Animation*>(component)->getSprite(elapsedTime));
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
        window.clear();
        for (auto* pDrawable : toDrawList) window.draw(*pDrawable);
        for (auto& sprite : sprites) window.draw(sprite);
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

private:
    sf::RenderWindow window;
    Scene scene;
    sf::Clock clock;
    KeySet pressedKeys;
    KeySet releasedKeys;
    std::vector<sf::Drawable*> toDrawList;
    std::vector<Task> toExecList;
    std::vector<sf::Sprite> sprites;
};

int main()
{
    const std::string windowConfigPath = "config\\settings.xml";
    Config config(windowConfigPath);
    Config::Window windowSettings = config.getWindowSettings();
    Game game(windowSettings);

    Config::Animation animationSettings = config.getAnimationSettings();

    Animation playerAnimation(animationSettings);
    playerAnimation.setPosition(windowSettings.w / 2.0f, windowSettings.h / 2.0f);

    KeyBindings playerControls;
    playerControls[sf::Keyboard::A] = [&](bool pressed)
    {
        auto type = (pressed) ? Animation::Type::WALK : Animation::Type::IDLE;
        playerAnimation.setType(type);
        playerAnimation.setDirection(Animation::Direction::LEFT);
    };

    playerControls[sf::Keyboard::D] = [&](bool pressed)
    {
        auto type = (pressed) ? Animation::Type::WALK : Animation::Type::IDLE;
        playerAnimation.setType(type);
        playerAnimation.setDirection(Animation::Direction::RIGHT);
    };

    {
        Entity playerEntity;
        playerEntity.components[Entity::ComponentType::BINDINGS] = &playerControls;
        playerEntity.components[Entity::ComponentType::ANIMATION] = &playerAnimation;

        game.AddEntity(std::move(playerEntity));
    }

    game.blockingRun();

    return 0;
}
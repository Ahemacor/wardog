#include <SFML/Graphics.hpp>
#include <tinyxml.h>
#include <tinystr.h>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <set>
#include <functional>

class Config
{
public:
    struct Window
    {
        int w = 400;
        int h = 400;
        std::string name = "Untitled";
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
        TiXmlElement* pWindowNode = hRoot.FirstChild("Windows").FirstChild().Element();
        windowSettings.name = pWindowNode->Attribute("name");
        pWindowNode->QueryIntAttribute("w", &windowSettings.w);
        pWindowNode->QueryIntAttribute("h", &windowSettings.h);
        return windowSettings;
    }

private:
    TiXmlDocument doc;
    TiXmlHandle hDoc;
    TiXmlElement* pElem;
    TiXmlHandle hRoot;
};

using KeySet = std::set<sf::Keyboard::Key>;
using Action = std::function<void()>;
using KeyBindings = std::map<sf::Keyboard::Key, Action>;

using Component = std::variant<sf::Drawable*, KeyBindings*>;

struct Entity
{
    enum class ComponentType
    {
        BINDINGS,
        DRAWABLE
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

        for (auto& entity : scene.entities)
        {
            for (auto& [type, component] : entity.components)
            {
                switch (type)
                {
                case Entity::ComponentType::BINDINGS:
                {
                    auto& binds = std::get<KeyBindings*>(component);
                    for (const auto& keyCode : pressedKeys)
                    {
                        auto findBind = binds->find(keyCode);
                        if (findBind != binds->cend())
                        {
                            auto& [bindKey, bindAction] = *findBind;
                            toExecList.push_back(bindAction);
                        }
                    }
                }
                    break;

                case Entity::ComponentType::DRAWABLE:
                    toDrawList.push_back(std::get<sf::Drawable*>(component));
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
    }

    void renderFrame()
    {
        window.clear();
        for (auto* pDrawable : toDrawList) window.draw(*pDrawable);
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
    std::vector<sf::Drawable*> toDrawList;
    std::vector<Action> toExecList;
};

int main()
{
    const std::string windowConfigPath = "config\\window.xml";
    Config config(windowConfigPath);
    Config::Window windowSettings = config.getWindowSettings();
    Game game(windowSettings);
    
    sf::RectangleShape rect;
    const float rectSize = 100.0f;
    rect.setSize(sf::Vector2f(rectSize, rectSize));
    rect.setOrigin(rectSize / 2.0f, rectSize / 2.0f);
    rect.setFillColor(sf::Color::Blue);
    rect.setPosition(windowSettings.w / 2.0f, windowSettings.h / 2.0f);

    Action fillRed = [&rect]() {rect.setFillColor(sf::Color::Red);};
    Action fillGreen = [&rect]() {rect.setFillColor(sf::Color::Green); };
    Action fillBlue = [&rect]() {rect.setFillColor(sf::Color::Blue); };
    Action fillWhite = [&rect]() {rect.setFillColor(sf::Color::White); };
    KeyBindings playerControls;
    playerControls[sf::Keyboard::W] = fillRed;
    playerControls[sf::Keyboard::S] = fillGreen;
    playerControls[sf::Keyboard::A] = fillBlue;
    playerControls[sf::Keyboard::D] = fillWhite;

    {
        Entity playerEntity;
        playerEntity.components[Entity::ComponentType::BINDINGS] = &playerControls;
        playerEntity.components[Entity::ComponentType::DRAWABLE] = &rect;

        game.AddEntity(std::move(playerEntity));
    }

    game.blockingRun();

    return 0;
}
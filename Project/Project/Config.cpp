#include "Config.h"
#include "CommonDefinitions.h"

std::unordered_map<std::string, Spritesheet> spriteSheetDescriptions;

const char* KeyNames[] =
{
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", // A-Z
    "Num0", "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9",
    "Escape", "LControl", "LShift", "LAlt", "LSystem", "RControl", "RShift", "RAlt", "RSystem", "Menu",
    "LBracket", "RBracket", "Semicolon", "Comma", "Period", "Quote", "Slash", "Backslash", "Tilde", "Equal", "Hyphen", // ();,.'/\`=-
    "Space", "Enter", "Backspace", "Tab",
    "PageUp", "PageDown", "End", "Home", "Insert",
    "Delete", "Add", "Subtract", "Multiply", "Divide",
    "Left", "Right", "Up", "Down", // arrows
    "Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5", "Numpad6", "Numpad7", "Numpad8", "Numpad9",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", 
    "Pause"
};

Action BuildSoundAction(const std::string& soundName)
{
    return [soundName](Entity& entity, bool pressed)
    {
        if (pressed)
        {
            PLAY_SOUND(soundName);
        }
        else
        {
            //STOP_SOUND(soundName);
        }
    };
}

Action BuildMoveAction(const b2Vec2& vector)
{
    return [vector](Entity& entity, bool pressed)
    {
        Component* pComponent = entity.getComponent(Component::Type::BODY);
        b2Body& body = *std::get<b2Body*>(pComponent->var);

        const float maxVel = 10.0f;
        const float forceScale = 20;

        if (pressed && body.GetLinearVelocity().Length() < maxVel)
        {
            const b2Vec2 scaledForce = { vector.x * forceScale, vector.y * forceScale };
            body.ApplyForceToCenter(scaledForce, true);
        }
        else
        {
            //body.SetLinearVelocity({ 0, 0 });
        }
    };
}

static constexpr const char* PATH_DELIMITER = "\\";

static constexpr const char* XML_TAG_SPRITE_SHEET = "Spritesheet";
static constexpr const char* XML_TAG_SPRITE_SHEET_TEXTURE = "texture";
static constexpr const char* XML_TAG_SPRITE_SHEET_NAME = "name";
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

Config& Config::getInstance(/*const std::string& configFile*/)
{
    static Config instance(MAIN_CONFIG_FILE);
    return instance;
}

Config::Config(const std::string& filepath)
: doc(filepath.c_str())
, hDoc(&doc)
, pElem(nullptr)
, hRoot(0)
{
    doc.LoadFile();
    pElem = hDoc.FirstChildElement().Element();
    hRoot = TiXmlHandle(pElem);
    LoadResoures(hRoot);
}

void Config::LoadResoures(TiXmlHandle rootHandle)
{
    static constexpr const char* XML_TAG_RESOURCES = "Resources";
    static constexpr const char* XML_TAG_RESOURCES_TYPE = "type";
    static constexpr const char* XML_TAG_RESOURCES_DIRECTORY = "directory";
    static constexpr const char* XML_TAG_RESOURCE_TYPE_TEXTURE = "Texture";
    static constexpr const char* XML_TAG_RESOURCE_TYPE_IMAGE = "Image";
    static constexpr const char* XML_TAG_RESOURCE_TYPE_SOUND = "Sound";
    static constexpr const char* XML_TAG_RESOURCE_TYPE_MUSIC = "Music";
    static constexpr const char* XML_TAG_RESOURCE_TYPE_FONT = "Font";
    static constexpr const char* XML_TAG_RESOURCE = "Resource";
    static constexpr const char* XML_TAG_RESOURCE_NAME = "name";
    static constexpr const char* XML_TAG_RESOURCE_EXT = "ext";

    TiXmlElement* resouresElem = rootHandle.FirstChild(XML_TAG_RESOURCES).Element();
    for (resouresElem; resouresElem != nullptr; resouresElem = resouresElem->NextSiblingElement())
    {
        const std::string elemName = resouresElem->Value();
        if (elemName != XML_TAG_RESOURCES) continue;

        const std::string typeString = resouresElem->Attribute(XML_TAG_RESOURCES_TYPE);
        const std::string directory = resouresElem->Attribute(XML_TAG_RESOURCES_DIRECTORY);

        Resource::Type type = Resource::Type::TEXTURE;
        if (typeString == XML_TAG_RESOURCE_TYPE_TEXTURE) type = Resource::Type::TEXTURE;
        else if (typeString == XML_TAG_RESOURCE_TYPE_SOUND)   type = Resource::Type::SOUND;
        else if (typeString == XML_TAG_RESOURCE_TYPE_MUSIC)   type = Resource::Type::MUSIC;
        else if (typeString == XML_TAG_RESOURCE_TYPE_FONT)    type = Resource::Type::FONT;
        else if (typeString == XML_TAG_RESOURCE_TYPE_IMAGE)   type = Resource::Type::IMAGE;
        else continue;

        TiXmlElement* resourceElem = resouresElem->FirstChild(XML_TAG_RESOURCE)->ToElement();
        for (resourceElem; resourceElem != nullptr; resourceElem = resourceElem->NextSiblingElement())
        {
            const std::string resourceName = resourceElem->Attribute(XML_TAG_RESOURCE_NAME);
            const std::string resourceExt = resourceElem->Attribute(XML_TAG_RESOURCE_EXT);
            const std::string resourcePath = directory + PATH_DELIMITER + resourceName + "." + resourceExt;

            LOAD_RESOURCE(type, resourceName, resourcePath);

            if (type == Resource::Type::TEXTURE)
            {
                bool tile = true;
                resourceElem->QueryBoolAttribute("tile", &tile);
                if (tile) TEXTURE(resourceName).setRepeated(true);
            }
        }
    }
}

Config::Window Config::getWindowSettings()
{
    Window windowSettings;
    TiXmlElement* pWindowNode = hRoot.FirstChild("GameWindow").FirstChild().Element();
    windowSettings.name = pWindowNode->Attribute("name");
    pWindowNode->QueryIntAttribute("w", &windowSettings.w);
    pWindowNode->QueryIntAttribute("h", &windowSettings.h);
    pWindowNode->QueryBoolAttribute("vSynch", &windowSettings.vSynch);
    windowSettings.icon = pWindowNode->Attribute("icon");
    return windowSettings;
}

void Config::loadAnimationSettings(TiXmlHandle rootHandle)
{
    TiXmlElement* spriteSheetElem = rootHandle.FirstChild(XML_TAG_SPRITE_SHEET).Element();
    for (spriteSheetElem; spriteSheetElem != nullptr; spriteSheetElem = spriteSheetElem->NextSiblingElement())
    {
        const std::string elemName = spriteSheetElem->Value();
        if (elemName != XML_TAG_SPRITE_SHEET) continue;

        Spritesheet spriteSheetInfo;
        spriteSheetInfo.texture = spriteSheetElem->Attribute(XML_TAG_SPRITE_SHEET_TEXTURE);
        spriteSheetInfo.name = spriteSheetElem->Attribute(XML_TAG_SPRITE_SHEET_NAME);
        spriteSheetElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_X_OFFSET, &spriteSheetInfo.x_offset);
        spriteSheetElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_Y_OFFSET, &spriteSheetInfo.y_offset);
        spriteSheetElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_WIDTH, &spriteSheetInfo.width);
        spriteSheetElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_HEIGHT, &spriteSheetInfo.height);
        spriteSheetElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_SCALE, &spriteSheetInfo.scale);

        TiXmlElement* animationElem = spriteSheetElem->FirstChild(XML_TAG_SPRITE_SHEET_ANIMATION)->ToElement();
        for (animationElem; animationElem != nullptr; animationElem = animationElem->NextSiblingElement())
        {
            Spritesheet::Animation animationInfo;
            animationInfo.name = animationElem->Attribute(XML_TAG_SPRITE_SHEET_ANIMATION_NAME);
            animationElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_ANIMATION_FRAMES, &animationInfo.num_frames);
            animationElem->QueryIntAttribute(XML_TAG_SPRITE_SHEET_ANIMATION_TIME, &animationInfo.ms_per_frame);
            for (TiXmlElement* animationChildElem = animationElem->FirstChildElement(); animationChildElem != nullptr; animationChildElem = animationChildElem->NextSiblingElement())
            {
                const char* directionName = animationChildElem->Value();
                const int row_index = animationChildElem->FirstAttribute()->IntValue();
                animationInfo.row_index[directionName] = row_index;
            }
            spriteSheetInfo.animations[animationInfo.name] = animationInfo;
        }

        spriteSheetDescriptions[spriteSheetInfo.name] = spriteSheetInfo;
    }
}

ControlActions Config::loadActions(Scene& scene, Entity& entity, const std::string& controllerName)
{
    std::map<std::string, ActionList> actions;
    ControlActions controller;
    LOG_INFO(std::string("controller: ") + controllerName);

    const std::string controllerPath = std::string("content\\config") + "\\" + controllerName + ".xml";
    TiXmlDocument controllerDoc(controllerPath.c_str());
    TiXmlHandle hControllerDoc(&controllerDoc);

    controllerDoc.LoadFile();
    TiXmlElement* pControllerElem = hControllerDoc.FirstChildElement().Element();
    const std::string controllerRootName = pControllerElem->Value();
    TiXmlHandle hControllerRoot = TiXmlHandle(pControllerElem);

    static constexpr const char* ACTIONS = "Actions";
    static constexpr const char* SOUND = "Sound";
    static constexpr const char* MOVE = "Move";

    static constexpr const char* CONTROLS = "Controls";
    static constexpr const char* ACTION = "action";
    static constexpr const char* NAME = "name";
    static constexpr const char* ENTITY = "entity";

    TiXmlElement* actionsElem = hControllerRoot.FirstChild(ACTIONS).Element();
    for (actionsElem; actionsElem != nullptr; actionsElem = actionsElem->NextSiblingElement())
    {
        const std::string actionsElemName = actionsElem->Value();
        if (actionsElemName != ACTIONS) continue;

        TiXmlElement* actionBundleElem = actionsElem->FirstChild()->ToElement();
        for (actionBundleElem; actionBundleElem != nullptr; actionBundleElem = actionBundleElem->NextSiblingElement())
        {
            const std::string actionBundleElemName = actionBundleElem->Value();

            TiXmlElement* actionElem = actionBundleElem->FirstChild()->ToElement();
            for (actionElem; actionElem != nullptr; actionElem = actionElem->NextSiblingElement())
            {
                const std::string actionName = actionElem->Value();
                if (actionName == SOUND)
                {
                    actions[actionBundleElemName].push_back(BuildSoundAction(actionElem->Attribute(NAME)));
                }
                else if (actionName == MOVE)
                {
                    b2Vec2 vector;
                    actionElem->QueryFloatAttribute("x", &vector.x);
                    actionElem->QueryFloatAttribute("y", &vector.y);
                    actions[actionBundleElemName].push_back(BuildMoveAction(vector));
                }

            }
        }
    }

    TiXmlElement* controlsElem = hControllerRoot.FirstChild(CONTROLS).Element();
    for (controlsElem; controlsElem != nullptr; controlsElem = controlsElem->NextSiblingElement())
    {
        const std::string controlsElemName = controlsElem->Value();
        if (controlsElemName != CONTROLS) continue;

        std::wstring controlsInfoText(L"”правление:\n----------------------------\n");

        TiXmlElement* controlElem = controlsElem->FirstChild()->ToElement();
        for (controlElem; controlElem != nullptr; controlElem = controlElem->NextSiblingElement())
        {
            const std::string controlName = controlElem->Value();
            const std::string controlActionName = controlElem->Attribute(ACTION);

            controlsInfoText += std::wstring(controlName.cbegin(), controlName.cend());
            controlsInfoText += L" -> ";
            controlsInfoText += std::wstring(controlActionName.cbegin(), controlActionName.cend());
            controlsInfoText += L"\n";

            int keyIndex = 0;
            for (const char* keyName : KeyNames)
            {
                if (controlName == keyName)
                {
                    controller[static_cast<sf::Keyboard::Key>(keyIndex)] = actions[controlActionName];
                }
                ++keyIndex;
            }

        }

        SHOW_TEXT(controlsInfoText);
    }
    return controller;
}

void Config::loadComponent(TiXmlElement* componentElem, Scene& scene, Entity& entity)
{
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_BODY = "Body";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_SHAPE = "Shape";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_SPRITE = "Sprite";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_ANIMAION = "Animation";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_CAMERA = "Camera";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE = "type";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_TEXTURE = "texture";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE_STATIC = "static";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_TYPE_DYNAMIC = "dynamic";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_WIDTH = "width";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_HEIGHT = "height";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_X = "x";
    static constexpr const char* XML_TAG_ENTITY_COMPONENT_Y = "y";

    float width, height, x, y;
    componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_WIDTH, &width);
    componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_HEIGHT, &height);
    componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_X, &x);
    componentElem->QueryFloatAttribute(XML_TAG_ENTITY_COMPONENT_Y, &y);

    const char* pTexture = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TEXTURE);
    std::string textureName = (pTexture != nullptr) ? pTexture : "";

    Component component;
    component.setPosition(x, y);

    const std::string componentName = componentElem->Value();
    if (componentName == XML_TAG_ENTITY_COMPONENT_BODY)
    {
        component.type = Component::Type::BODY;
        const std::string componentTypeString = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TYPE);
        const b2BodyType bodyType = (componentTypeString == XML_TAG_ENTITY_COMPONENT_TYPE_DYNAMIC) ? b2_dynamicBody : b2_staticBody;

        b2BodyDef bodyDef;
        bodyDef.type = bodyType;
        bodyDef.position.Set(pixelToMeter(x), pixelToMeter(y));
        bodyDef.angularDamping = 15.0f;
        bodyDef.linearDamping = 10.0f;
        b2Body* body = scene.world.CreateBody(&bodyDef);

        b2PolygonShape boxShape;
        boxShape.SetAsBox(pixelToMeter(width) / 2, pixelToMeter(height) / 2);

        b2FixtureDef fixture;
        fixture.shape = &boxShape;
        fixture.density = 1.0f;
        fixture.friction = 0.8f;
        body->CreateFixture(&fixture);
        body->SetFixedRotation(true);
        component.var = body;
    }
    else if (componentName == XML_TAG_ENTITY_COMPONENT_SHAPE)
    {
        component.type = Component::Type::SHAPE;
        component.var = sf::RectangleShape({ width, height });
        sf::RectangleShape& rect = std::get<sf::RectangleShape>(component.var);
        rect.setOrigin(width / 2, height / 2);
        rect.setPosition(x, y);
        if (!textureName.empty())
        {
            rect.setTexture(&TEXTURE(textureName));
        }
    }
    else if (componentName == XML_TAG_ENTITY_COMPONENT_SPRITE)
    {
        component.type = Component::Type::SPRITE;
        const sf::IntRect intRect({0, 0}, { (int)width, (int)height });
        component.var = sf::Sprite(TEXTURE(textureName));
        sf::Sprite& sprite = std::get<sf::Sprite>(component.var);
        sprite.setTextureRect(intRect);
        sprite.setOrigin(width / 2, height / 2);
        sprite.setPosition(x, y);
    }
    else if (componentName == XML_TAG_ENTITY_COMPONENT_ANIMAION)
    {
        component.type = Component::Type::ANIMATION;
        const char* pAnimationName = componentElem->Attribute("name");
        if (pAnimationName != nullptr)
        {
            component.var = Animation(spriteSheetDescriptions[pAnimationName]);
        }
    }
    else if (componentName == XML_TAG_ENTITY_COMPONENT_CAMERA)
    {
        component.type = Component::Type::CAMERA;
        component.var = sf::View({ x, y }, { width, height });
    }
    else if (componentName == "Controller")
    {
        component.type = Component::Type::CONTROLLER;
        const char* pControllerName = componentElem->Attribute("name");
        if (pControllerName != nullptr)
        {
            component.var = loadActions(scene, entity, pControllerName);
        }
    }

    entity.components.push_back(component);
}

void Config::loadEntity(TiXmlElement* entityElem, Scene& scene)
{
    static constexpr const char* XML_TAG_ENTITY_NAME = "name";

    Entity entity;
    entity.name = entityElem->Attribute(XML_TAG_ENTITY_NAME);

    LOG_INFO(std::string("entity: ") + entity.name);

    TiXmlElement* componentElem = entityElem->FirstChild()->ToElement();
    for (componentElem; componentElem != nullptr; componentElem = componentElem->NextSiblingElement())
    {
        loadComponent(componentElem, scene, entity);
    }

    scene.sceneGraph.push_back(entity);
}

void Config::loadEntities(TiXmlHandle rootHandle, Scene& scene)
{
    auto sceneHandle = rootHandle.FirstChild("Scene");
    TiXmlElement* entityElem = sceneHandle.FirstChild("Entity").Element();
    for (entityElem; entityElem != nullptr; entityElem = entityElem->NextSiblingElement())
    {
        const std::string elemName = entityElem->Value();
        if (elemName == "Entity")
        {
            loadEntity(entityElem, scene);
        }
    }
}

void Config::loadMenu(TiXmlHandle menuHandle, Scene& scene)
{
    Menu menu;
    TiXmlElement* menuElem = menuHandle.Element();
    menu.name = menuElem->Attribute("name");
    menu.fontName = menuElem->Attribute("font");

    float x, y;
    menuElem->QueryFloatAttribute("x", &x);
    menuElem->QueryFloatAttribute("y", &y);
    menu.setPosition(x, y);

    menuElem->QueryIntAttribute("charSize", &menu.charSize);

    TiXmlElement* optionElem = menuHandle.FirstChild("Option").Element();
    for (optionElem; optionElem != nullptr; optionElem = optionElem->NextSiblingElement())
    {
        const std::string optionName = optionElem->Value();
        if (optionName == "Option")
        {
            Menu::Option option;
            option.type = Menu::Option::Type::TEXT_OPTION;
            option.name = optionElem->Attribute("name");
            option.action = optionElem->Attribute("action");

            const char* args_str = optionElem->Attribute("args");
            if (args_str != nullptr)
            {
                std::string args_string = args_str;
                auto begin = args_string.cbegin();
                auto end = args_string.cend();
                while (begin != end)
                {
                    auto newEnd = std::find(begin, end, ' ');
                    option.args.push_back(std::string(begin, newEnd));
                    begin = newEnd;
                }
            }

            option.text.setFont(FONT(menu.fontName));
            menu.options.push_back(option);
        }
    }
    bool isMenuActive = false;
    menuElem->QueryBoolAttribute("active", &isMenuActive);

    scene.allMenu[menu.name] = menu;
    if (isMenuActive)
    {
        scene.menuStack.push(menu);
    }
}

void Config::loadUI(TiXmlHandle rootHandle, Scene& scene)
{
    auto sceneHandle = rootHandle.FirstChild("UI");
    TiXmlElement* menuElem = sceneHandle.FirstChild("Menu").Element();
    for (menuElem; menuElem != nullptr; menuElem = menuElem->NextSiblingElement())
    {
        const std::string menuName = menuElem->Value();
        if (menuName == "Menu")
        {
            loadMenu(menuElem, scene);
        }
    }
}

std::vector<std::string> Config::readLevelList()
{
    std::vector<std::string> levels;

    TiXmlElement* levelsElem = hRoot.FirstChild("Levels").Element();
    const std::string directory = levelsElem->Attribute("directory");
    levelDir = directory;
    for (TiXmlElement* levelElem = levelsElem->FirstChild()->ToElement();
        levelElem != nullptr;
        levelElem = levelElem->NextSiblingElement())
    {
        const std::string elemName = levelElem->Value();
        const std::string levelName = levelElem->Attribute("name");
        if (elemName == "StartLevel")
        {
            startLevel = levelName;
        }

        levels.push_back(levelName);
    }

    return levels;
}

std::string Config::getStartLevelName()
{
    readLevelList();
    return startLevel;
}

void Config::loadLevel(const std::string& levelName, Scene& scene)
{
    LOG_INFO(std::string("level: ") + levelName);
    readLevelList();
    const std::string levelPath = levelDir + "\\" + levelName + ".xml";
    TiXmlDocument levelDoc(levelPath.c_str());
    TiXmlHandle hLevelDoc(&levelDoc);

    levelDoc.LoadFile();
    TiXmlElement* pLevelElem = hLevelDoc.FirstChildElement().Element();
    const std::string levelRootName = pLevelElem->Value();
    TiXmlHandle hLevelRoot = TiXmlHandle(pLevelElem);
    LoadResoures(hLevelRoot);
    loadAnimationSettings(hLevelRoot);
    loadPlaylist(hLevelRoot);
    loadEntities(hLevelRoot, scene);
    loadUI(hLevelRoot, scene);

    currentLevel = levelName;
   
    scene.playlist = musicPlaylist;
}

void Config::loadPlaylist(TiXmlHandle rootHandle)
{
    musicPlaylist.clear();
    auto sceneHandle = rootHandle.FirstChild("Playlist");
    TiXmlElement* playlistElem = sceneHandle.FirstChild("Music").Element();
    for (playlistElem; playlistElem != nullptr; playlistElem = playlistElem->NextSiblingElement())
    {
        const std::string playlistElemName = playlistElem->Value();
        if (playlistElemName == "Music")
        {
            const std::string musicName = playlistElem->Attribute("name");
            musicPlaylist.push_back(musicName);
        }
    }
}
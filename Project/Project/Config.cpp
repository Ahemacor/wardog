#include "Config.h"
#include "CommonDefinitions.h"

ControlActions g_controls;

std::unordered_map<std::string, ActionList> g_actions;

const char* KeyNames[] =
{
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "Num0",
    "Num1",
    "Num2",
    "Num3",
    "Num4",
    "Num5",
    "Num6",
    "Num7",
    "Num8",
    "Num9",
    "Escape",
    "LControl",
    "LShift",
    "LAlt",
    "LSystem",
    "RControl",
    "RShift",
    "RAlt",
    "RSystem",
    "Menu",
    "LBracket",
    "RBracket",
    "Semicolon",
    "Comma",
    "Period",
    "Quote",
    "Slash",
    "Backslash",
    "Tilde",
    "Equal",
    "Hyphen",
    "Space",
    "Enter",
    "Backspace",
    "Tab",
    "PageUp",
    "PageDown",
    "End",
    "Home",
    "Insert",
    "Delete",
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "Left",
    "Right",
    "Up",
    "Down",
    "Numpad0",
    "Numpad1",
    "Numpad2",
    "Numpad3",
    "Numpad4",
    "Numpad5",
    "Numpad6",
    "Numpad7",
    "Numpad8",
    "Numpad9",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "Pause"
};

Action BuildSoundAction(const std::string& soundName)
{
    return [soundName](bool pressed)
    {
        if (pressed)
        {
            PLAY_SOUND(soundName);
        }
        else
        {
            STOP_SOUND(soundName);
        }
    };
}

Action BuildMoveAction(const std::string& entityName, const b2Vec2& vector)
{
    return [entityName, vector](bool pressed)
    {
        Entity& entity = GAME_INSTANCE.scene.GetEntityRefByName(entityName);
        b2Body& body = entity.GetRectangleBody();

        if (pressed)
        {
            body.SetLinearVelocity(vector);
            entity.cameraFocus = true;
        }
        else
        {
            body.SetLinearVelocity({ 0, 0 });
            entity.cameraFocus = false;
        }
    };
}

static constexpr const char* PATH_DELIMITER = "\\";

static constexpr const char* XML_TAG_SPRITE_SHEET = "Spritesheet";
static constexpr const char* XML_TAG_SPRITE_SHEET_TEXTURE = "texture";
static constexpr const char* XML_TAG_STRETCH_TEXTURE = "stretch";
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
    LoadResoures();
}

void Config::LoadResoures()
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

    TiXmlElement* resouresElem = hRoot.FirstChild(XML_TAG_RESOURCES).Element();
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
                bool stretchTexture = true;
                resourceElem->QueryBoolAttribute(XML_TAG_STRETCH_TEXTURE, &stretchTexture);
                if (stretchTexture == false)
                {
                    TEXTURE(resourceName).setRepeated(true);
                }
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

std::vector<Config::Spritesheet> Config::getAnimationSettings()
{
    std::vector<Spritesheet> spriteSheets;

    TiXmlElement* spriteSheetElem = hRoot.FirstChild(XML_TAG_SPRITE_SHEET).Element();
    for (spriteSheetElem; spriteSheetElem != nullptr; spriteSheetElem = spriteSheetElem->NextSiblingElement())
    {
        const std::string elemName = spriteSheetElem->Value();
        if (elemName != XML_TAG_SPRITE_SHEET) continue;

        Config::Spritesheet spriteSheetInfo;
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
            Config::Spritesheet::Animation animationInfo;
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

        spriteSheets.push_back(std::move(spriteSheetInfo));
    }

    return spriteSheets;
}

void Config::initActions()
{
    static constexpr const char* ACTIONS = "Actions";
    static constexpr const char* SOUND = "Sound";
    static constexpr const char* MOVE = "Move";

    static constexpr const char* CONTROLS = "Controls";
    static constexpr const char* ACTION = "action";
    static constexpr const char* NAME = "name";
    static constexpr const char* ENTITY = "entity";

    TiXmlElement* actionsElem = hRoot.FirstChild(ACTIONS).Element();
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
                    g_actions[actionBundleElemName].push_back(BuildSoundAction(actionElem->Attribute(NAME)));
                }
                else if (actionName == MOVE)
                {
                    b2Vec2 vector;
                    actionElem->QueryFloatAttribute("x", &vector.x);
                    actionElem->QueryFloatAttribute("y", &vector.y);
                    g_actions[actionBundleElemName].push_back(BuildMoveAction(actionElem->Attribute(ENTITY), vector));
                }

            }
        }
    }

    TiXmlElement* controlsElem = hRoot.FirstChild(CONTROLS).Element();
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
                    g_controls[static_cast<sf::Keyboard::Key>(keyIndex)] = g_actions[controlActionName];
                }
                ++keyIndex;
            }

        }

        SHOW_TEXT(controlsInfoText);
    }
}

std::vector<Config::Entity> Config::getEntityDescriptions()
{
    static constexpr const char* XML_TAG_ENTITY = "Entity";
    static constexpr const char* XML_TAG_ENTITY_NAME = "name";
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
            else if (componentName == XML_TAG_ENTITY_COMPONENT_ANIMAION)
            {
                componentInfo.type = Entity::Component::Type::ANIMATION;
                const char* pAnimationName = componentElem->Attribute("name");
                if (pAnimationName != nullptr)
                {
                    componentInfo.animation = pAnimationName;
                }
            }
            else if (componentName == XML_TAG_ENTITY_COMPONENT_CAMERA)
            {
                componentInfo.type = Entity::Component::Type::CAMERA;
            }
            else if (componentName == XML_TAG_ENTITY_COMPONENT_SPRITE)
            {
                componentInfo.type = Entity::Component::Type::SPRITE;
            }

            const char* pTexture = componentElem->Attribute(XML_TAG_ENTITY_COMPONENT_TEXTURE);
            if (pTexture != nullptr)
            {
                componentInfo.texture = pTexture;
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
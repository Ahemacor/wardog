#pragma once

#include <tinyxml.h>
#include <box2d/box2d.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <map>
#include <set>
#include <SFML/Window.hpp>

class Config
{
public:
    struct Window
    {
        int w = 400;
        int h = 400;
        std::string name = "Untitled";
        bool vSynch = false;
        std::string icon;
    };

    struct Spritesheet
    {
        std::string name;
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
                SHAPE,
                SPRITE,
                ANIMATION,
                CAMERA
            };
            Type type;

            b2BodyType bodyType;
            std::string texture;
            std::string animation;
            float width;
            float height;
            float x;
            float y;
        };
        std::vector<Component> components;
    };

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static Config& getInstance(/*const std::string& configFile*/);

    void LoadResoures();

    Window getWindowSettings();

    std::vector<Spritesheet> getAnimationSettings();

    void initActions();

    std::vector<Entity> getEntityDescriptions();

private:
    Config(const std::string& filepath);

    TiXmlDocument doc;
    TiXmlHandle hDoc;
    TiXmlElement* pElem;
    TiXmlHandle hRoot;
};

using Action = std::function<void(bool)>;
using ActionList = std::vector<Action>;
using KeySet = std::set<sf::Keyboard::Key>;
using Task = std::function<void()>;
using ControlActions = std::map<sf::Keyboard::Key, ActionList>;

extern ControlActions g_controls;
extern std::unordered_map<std::string, ActionList> g_actions;
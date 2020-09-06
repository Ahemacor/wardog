#pragma once

#include "Entity.h"
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

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static Config& getInstance(/*const std::string& configFile*/);

    void LoadResoures(TiXmlHandle rootHandle);

    Window getWindowSettings();

    void loadAnimationSettings(TiXmlHandle rootHandle);

    void initActions();

    std::vector<std::string> readLevelList();
    std::string getStartLevelName();

    void loadLevel(const std::string& levelName);

    std::vector<std::string> musicPlaylist;

    std::string currentLevel;

private:
    Config(const std::string& filepath);
    void loadEntities(TiXmlHandle rootHandle);
    Entity_ loadEntity(TiXmlElement* entityElem);
    Entity_::Component loadComponent(TiXmlElement* componentElem);

    void loadPlaylist(TiXmlHandle rootHandle);

    TiXmlDocument doc;
    TiXmlHandle hDoc;
    TiXmlElement* pElem;
    TiXmlHandle hRoot;

    std::string levelDir;
    std::string startLevel;

};

using Action = std::function<void(bool)>;
using ActionList = std::vector<Action>;
using KeySet = std::set<sf::Keyboard::Key>;

using ControlActions = std::map<sf::Keyboard::Key, ActionList>;
extern ControlActions g_controls;
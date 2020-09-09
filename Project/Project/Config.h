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

struct Scene;
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

    ControlActions loadActions(Scene& scene, Entity& entity, const std::string& controllerName);

    std::vector<std::string> readLevelList();
    std::string getStartLevelName();

    void loadLevel(const std::string& levelName, Scene& scene);

    std::vector<std::string> musicPlaylist;

    std::string currentLevel;

private:
    Config(const std::string& filepath);
    void loadEntities(TiXmlHandle rootHandle, Scene& scene);
    void loadEntity(TiXmlElement* entityElem, Scene& scene);
    void loadComponent(TiXmlElement* componentElem, Scene& scene, Entity& entity);

    void loadMenu(TiXmlHandle menuHandle, Scene& scene);
    void loadUI(TiXmlHandle rootHandle, Scene& scene);

    void loadPlaylist(TiXmlHandle rootHandle);

    TiXmlDocument doc;
    TiXmlHandle hDoc;
    TiXmlElement* pElem;
    TiXmlHandle hRoot;

    std::string levelDir;
    std::string startLevel;

};

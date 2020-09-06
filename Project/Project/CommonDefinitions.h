#pragma once

#include "Resource.h"
#include "AudioSystem.h"
#include "UiManager.h"
#include "Config.h"
#include "Animation.h"
#include "Scene.h"
#include "Entity.h"
#include "Game.h"

// Conversions
constexpr float SCALE_FACTOR = 100.0f;
static constexpr float pixelToMeter(int pixels) { return pixels / SCALE_FACTOR; }
static constexpr int meterToPixel(float meters) { return static_cast<int>(meters * SCALE_FACTOR); }
static constexpr float degreeToRadian(float degrees) { return degrees / 180 * M_PI; }
static constexpr float radianToDegree(float radians) { return radians * 180 * M_PI; }

// Resource
#define TEXTURE(name) (g_resources[(name)].get<Resource::Type::TEXTURE>())
#define IMAGE(name) (g_resources[(name)].get<Resource::Type::IMAGE>())
#define SOUND(name) (g_resources[(name)].get<Resource::Type::SOUND>())
#define MUSIC(name) (g_resources[(name)].get<Resource::Type::MUSIC>())
#define FONT(name) (g_resources[(name)].get<Resource::Type::FONT>())

#define LOAD_RESOURCE(TYPE, NAME, PATH) (g_resources[(NAME)].load((TYPE), (NAME), (PATH)))

// AudioSystem
#define PLAY_SOUND(NAME) (AudioSystem::getInstance().playSound((NAME)))
#define STOP_SOUND(NAME) (AudioSystem::getInstance().stopSound((NAME)))
#define PLAY_MUSIC(NAME) (AudioSystem::getInstance().playMusic((NAME)))

// UiManager
#define UI_INSTANCE UiManager::getInstance()
#define LOG_INFO(logText) (UiManager::getInstance().log((logText), UiManager::LogType::INFO))
#define LOG_ERROR(logText) (UiManager::getInstance().log((logText), UiManager::LogType::ERROR))

#define SHOW_TEXT(TEXT) (UiManager::getInstance().AddStaticText((TEXT)))

#define UPDATE_UI() (UiManager::getInstance().updateLogs())

#define COUNT_RENDER(TIME) (UiManager::getInstance().calculateFps((TIME)))

// Config
#define MAIN_CONFIG_FILE "content\\config\\settings.xml"
#define WINDOW_CONFIG Config::getInstance().getWindowSettings()
#define BUILD_ACTIONS_FROM_CONFIG() (Config::getInstance().initActions())
#define LOAD_ENTITIES() Config::getInstance().loadEntities();
#define CONFIG (Config::getInstance())

// Game
#define GAME_INIT() (Game::getInstance())
#define GAME_INSTANCE (Game::getInstance())
#define GAME_START() (Game::getInstance().blockingRun())



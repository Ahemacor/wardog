#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <unordered_map>
#include <functional>
#include <queue>

struct Menu : public sf::Drawable, public sf::Transformable
{
    struct Option
    {
        enum class Type
        {
            TEXT_OPTION
        };

        Type type;
        std::string name;
        std::string action;
        std::vector<std::string>args;
        sf::Text text;
    };

    void handleEvent(const sf::Event& event);
    void update(const sf::Time& elapsedTime);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::string fontName = "pixel_font";
    int charSize = 30;
    std::string name = "Test Menu Title";
    sf::Text text;

    std::vector<Option> options;
    int selected = -1;
};

class UiManager : public sf::Drawable
{
public:
    enum class LogType
    {
        INFO,
        ERROR
    };

    UiManager(const UiManager&) = delete;
    UiManager& operator=(const UiManager&) = delete;

    UiManager(UiManager&&) = delete;
    UiManager& operator=(UiManager&&) = delete;

    void handleEvent(const sf::Event& event);

    static UiManager& getInstance();

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void AddStaticText(const std::wstring& textString);
    void clearStaticText();
    void calculateFps(const sf::Time& elapsedTime);
    void update(const sf::Time& elapsedTime);

    void log(const std::string logString, LogType type = LogType::INFO);
    void log(const std::wstring logString, LogType type = LogType::INFO);

private:
    UiManager(float width, float height);

    sf::Clock updateClock;

    float win_width;
    float win_height;
    std::string fontName = "pixel_font";
    int charSize = 30;

    sf::Vector2f staticTextPosition = { win_width - 300, 30 };
    std::vector<sf::Text> uiStaticText;

    sf::Vector2f fpsTextPosition = { win_width - 300, 0 };
    sf::Text fpsText;

    int numLogLines = (win_width / 2) / charSize;
    sf::Vector2f logTextPosition = { 10, 10 };
    std::deque<sf::Text> logQueueText;
};

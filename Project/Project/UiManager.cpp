#include "UiManager.h"
#include "CommonDefinitions.h"

UiManager& UiManager::getInstance()
{
    const auto& windowConfig = WINDOW_CONFIG;
    static UiManager instance(windowConfig.w, windowConfig.h);
    return instance;
}

UiManager::UiManager(float width, float height) : win_width(width), win_height(height) {}

void UiManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (const auto& text : uiStaticText)
    {
        target.draw(text, states);
    }

    target.draw(fpsText, states);

    for (const auto& logText : logQueueText)
    {
        target.draw(logText, states);
    }
}

void UiManager::AddStaticText(const std::wstring& textString)
{
    const sf::Font& font = FONT(fontName);
    sf::Text textToDraw(textString.c_str(), font, charSize);
    textToDraw.setPosition(staticTextPosition);
    uiStaticText.push_back(std::move(textToDraw));
}

void UiManager::calculateFps(const sf::Time& elapsedTime)
{
    if (fpsText.getFont() == nullptr)
    {
        const sf::Font& font = FONT(fontName);
        fpsText.setFont(font);
        fpsText.setCharacterSize(charSize);
        fpsText.setPosition(fpsTextPosition);
    }

    static sf::Time lastUpdate;
    if (lastUpdate + sf::seconds(1.0f) <= updateClock.getElapsedTime())
    {
        const int fpsNum = sf::seconds(1.0f) / elapsedTime;
        fpsText.setString(std::wstring(L"FPS: ") + std::to_wstring(fpsNum)
            + L" Frame time: " + std::to_wstring(elapsedTime.asMicroseconds()));
        lastUpdate = updateClock.getElapsedTime();
    }
}

void UiManager::updateLogs()
{
    static sf::Time lastUpdate;
    if (lastUpdate + sf::seconds(0.05f) <= updateClock.getElapsedTime())
    {
        for (sf::Text& logText : logQueueText)
        {
            sf::Color color = logText.getFillColor();
            color.a--;
            if (color.a > 0)
            {
                logText.setFillColor(color);
            }
        }

        lastUpdate = updateClock.getElapsedTime();
    }
}

void UiManager::log(const std::string logString, LogType type)
{
    log(std::wstring(logString.cbegin(), logString.cend()), type);
}

void UiManager::log(const std::wstring logString, LogType type)
{
    const sf::Font& font = FONT(fontName);
    sf::Text textToDraw(logString, font, charSize);
    textToDraw.setPosition(logTextPosition.x, logTextPosition.y + (charSize * logQueueText.size()));
    textToDraw.setFillColor((type == LogType::ERROR) ? sf::Color::Red : sf::Color::Green);
    logQueueText.push_front(textToDraw);

    if (logQueueText.size() > numLogLines)
    {
        logQueueText.pop_back();
        for (sf::Text& logText : logQueueText)
        {
            logText.move(0, -charSize);
        }
    }
}
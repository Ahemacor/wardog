#include "UiManager.h"
#include "CommonDefinitions.h"

void Menu::handleEvent(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::KeyPressed:
        break;

    case sf::Event::KeyReleased:
        if (event.key.code == sf::Keyboard::Key::Up)
        {
            PLAY_SOUND("select");
            --selected;
            if (selected < 0) selected = options.size() - 1;
        }
        else if (event.key.code == sf::Keyboard::Key::Down)
        {
            PLAY_SOUND("select");
            ++selected;
            if (selected >= options.size()) selected = 0;
        }
        else if (selected >=0 && event.key.code == sf::Keyboard::Key::Enter)
        {
            LOG_INFO(options[selected].action);
            PLAY_SOUND("activate");
            GAME_INSTANCE.exec(options[selected].action, options[selected].args);
        }
        break;

    case sf::Event::MouseMoved:
        {
            const sf::Vector2f mousePos = { (float)event.mouseMove.x,  (float)event.mouseMove.y };
            const sf::FloatRect mouseRect = { mousePos, {1, 1} };
            for (int idx = 0; idx < options.size(); ++idx)
            {
                const Option& option = options[idx];
                if (selected != idx && option.text.getGlobalBounds().intersects(mouseRect))
                {
                    PLAY_SOUND("select");
                    selected = idx;
                }
            }
        }
        break;

    case sf::Event::MouseButtonPressed:
        if (event.mouseButton.button == sf::Mouse::Button::Left)
        {
            const sf::Vector2f mousePos = { (float)event.mouseButton.x,  (float)event.mouseButton.y };
            const sf::FloatRect mouseRect = { mousePos, {1, 1} };
            for (int idx = 0; idx < options.size(); ++idx)
            {
                const Option& option = options[idx];
                if (option.text.getGlobalBounds().intersects(mouseRect))
                {
                    LOG_INFO(options[selected].action);
                    PLAY_SOUND("activate");
                    GAME_INSTANCE.exec(options[selected].action, options[selected].args);
                }
            }
        }
        break;

    default:
        break;
    }
}

void Menu::update(const sf::Time& elapsedTime)
{
    const sf::Color SELECTED_COLOR(255, 255, 255, 255);
    const sf::Color DEFAULT_COLOR(255, 255, 255, 150);

    const sf::Font& font = FONT(fontName);
    text.setFont(font);
    text.setCharacterSize(charSize);
    text.setString(name);
    text.setPosition(getPosition());
    text.setFillColor(sf::Color::White);

    int movement = charSize;
    for (int idx = 0; idx < options.size(); ++idx)
    {
        auto& option = options[idx];
        option.text.setFont(font);
        option.text.setCharacterSize(charSize);
        option.text.setString(option.name);
        option.text.setPosition(getPosition());
        option.text.setFillColor((idx == selected) ? SELECTED_COLOR : DEFAULT_COLOR);
        option.text.setFillColor((idx == selected) ? SELECTED_COLOR : DEFAULT_COLOR);
        option.text.move(0, movement);
        movement += charSize;
    }
}

void Menu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(text, states);
    for (auto& option : options)
    {
        target.draw(option.text, states);
    }
}

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

    if (!GAME_INSTANCE.scene.menuStack.empty())
    {
        target.draw(GAME_INSTANCE.scene.menuStack.top());
    }
}

void UiManager::AddStaticText(const std::wstring& textString)
{
    const sf::Font& font = FONT(fontName);
    sf::Text textToDraw(textString.c_str(), font, charSize);
    textToDraw.setPosition(staticTextPosition);
    uiStaticText.push_back(std::move(textToDraw));
}

void UiManager::clearStaticText()
{
    uiStaticText.clear();
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

void UiManager::update(const sf::Time& elapsedTime)
{
    static sf::Time lastUpdate;
    if (lastUpdate + sf::seconds(0.1f) <= updateClock.getElapsedTime())
    {
        int numExpired = 0;
        for (sf::Text& logText : logQueueText)
        {
            sf::Color color = logText.getFillColor();
            color.a--;
            if (color.a > 0)
            {
                logText.setFillColor(color);
            }
            else
            {
                ++numExpired;
            }
        }

        while (numExpired > 0)
        {
            logQueueText.pop_back();
            for (sf::Text& logText : logQueueText)
            {
                logText.move(0, -charSize);
            }
            --numExpired;
        }

        lastUpdate = updateClock.getElapsedTime();
    }

    if (!GAME_INSTANCE.scene.menuStack.empty())
    {
        GAME_INSTANCE.scene.menuStack.top().update(elapsedTime);
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

void UiManager::handleEvent(const sf::Event& event)
{
    if (!GAME_INSTANCE.scene.menuStack.empty())
    {
        GAME_INSTANCE.scene.menuStack.top().handleEvent(event);
    }
}
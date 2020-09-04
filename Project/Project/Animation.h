#pragma once

#include <SFML/Graphics.hpp>
#include "Config.h"
#include <vector>

class Animation
{
public:
    enum class Type
    {
        IDLE,
        WALK
    };

    enum class Direction
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    Animation(const Config::Spritesheet& animationSettings);

    sf::Sprite getSprite(Type type, Direction direction, sf::Time elapsedTime);

    sf::Sprite getSprite(sf::Time elapsedTime);

    void setPosition(float xpos, float ypos);

    void setType(Type type) { this->type = type; }
    Type getType() { return type; }

    void setDirection(Direction direction) { this->direction = direction; }
    Direction getDirection() { direction; }

    std::string getName() { return settings.name; }

private:

    int getColumnId(int msPerFrame, int numOfFrames, sf::Time elapsedTime);

    Config::Spritesheet settings;

    sf::Sprite sprite;
    Type type = Type::IDLE;
    Direction direction = Direction::RIGHT;

    sf::Time time;
};

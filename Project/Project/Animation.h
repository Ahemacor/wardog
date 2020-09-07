#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>

struct Spritesheet;

class Animation : public sf::Drawable
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

    Animation(const Spritesheet& spriteSheetDescr);

    void update(b2Vec2 velocity);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void setPosition(float xpos, float ypos);

    void setType(Type type) { this->type = type; }
    Type getType() { return type; }

    void setDirection(Direction direction) { this->direction = direction; }
    Direction getDirection() { direction; }

    std::string getName();

private:

    int getColumnId(int msPerFrame, int numOfFrames);

    sf::Sprite sprite;
    Type type = Type::IDLE;
    Direction direction = Direction::RIGHT;

    sf::Time elapsedTime;
    sf::Clock clock;
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
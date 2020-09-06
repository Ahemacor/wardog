#include "Animation.h"
#include "CommonDefinitions.h"

Spritesheet settings;

static constexpr const char* AnimationNames[] =
{
    "Idle",
    "Walk"
};

static constexpr const char* DirectionNames[] =
{
    "Up",
    "Down",
    "Left",
    "Right"
};

Animation::Animation(const Spritesheet& spriteSheetDescr)
{
    settings = spriteSheetDescr;
    sprite.setOrigin(settings.width / 2.0f, settings.height / 2.0f);
    sprite.setScale((float)settings.scale, (float)settings.scale);
    sprite.setTexture(TEXTURE(settings.texture));
}

sf::Sprite Animation::getSprite(Type type, Direction direction, sf::Time elapsedTime)
{
    const char* directionName = DirectionNames[static_cast<int>(direction)];
    const char* animationName = AnimationNames[static_cast<int>(type)];

    int row_id = 0;
    int column_id = 0;

    auto& animationInfo = settings.animations[animationName];
    row_id = animationInfo.row_index[directionName];
    column_id = getColumnId(animationInfo.ms_per_frame, animationInfo.num_frames, elapsedTime);

    sf::IntRect newTectureRect(settings.x_offset + (column_id * settings.width),
        settings.y_offset + (row_id * settings.height),
        settings.width, settings.height);

    sprite.setTextureRect(newTectureRect);

    return sprite;
}

sf::Sprite Animation::getSprite(sf::Time elapsedTime)
{
    return getSprite(type, direction, elapsedTime);
}

void Animation::setPosition(float xpos, float ypos)
{
    sprite.setPosition(xpos, ypos);
}

int Animation::getColumnId(int msPerFrame, int numOfFrames, sf::Time elapsedTime)
{
    time += elapsedTime;
    sf::Time fullAnimDuration = sf::milliseconds(msPerFrame * numOfFrames);
    if (time > fullAnimDuration) time %= fullAnimDuration;
    return time.asMilliseconds() / msPerFrame;
}

std::string Animation::getName() 
{ 
    return settings.name;
}
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

void Animation::update(b2Vec2 velocity)
{
    const float bias = 0.1f;
    const float velVectLen = velocity.Length();

    if (velVectLen > bias)
    {
        setType(Animation::Type::WALK);
        float absX = std::fabsf(velocity.x);
        float absY = std::fabsf(velocity.y);
        if (absX >= absY)
        {
            if (velocity.x <= -bias)     setDirection(Animation::Direction::LEFT);
            else if (velocity.x >= bias) setDirection(Animation::Direction::RIGHT);
        }
        else
        {
            if (velocity.y <= -bias)     setDirection(Animation::Direction::UP);
            else if (velocity.y >= bias) setDirection(Animation::Direction::DOWN);
        }
        const float scaleFactor = std::fminf(velVectLen, 1.0f);
        elapsedTime += clock.restart() * scaleFactor;
    }
    else
    {
        setType(Animation::Type::IDLE);
        elapsedTime += clock.restart();
    }

    const char* directionName = DirectionNames[static_cast<int>(direction)];
    const char* animationName = AnimationNames[static_cast<int>(type)];

    auto& animationInfo = settings.animations[animationName];
    const int row_id = animationInfo.row_index[directionName];
    const int column_id = getColumnId(animationInfo.ms_per_frame, animationInfo.num_frames);

    sf::IntRect newTectureRect(settings.x_offset + (column_id * settings.width),
        settings.y_offset + (row_id * settings.height),
        settings.width, settings.height);

    sprite.setTextureRect(newTectureRect);
}

void Animation::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(sprite, states);
}

int Animation::getColumnId(int msPerFrame, int numOfFrames)
{
    sf::Time fullAnimDuration = sf::milliseconds(msPerFrame * numOfFrames);
    if (elapsedTime > fullAnimDuration) elapsedTime %= fullAnimDuration;
    return elapsedTime.asMilliseconds() / msPerFrame;
}

std::string Animation::getName() 
{ 
    return settings.name;
}
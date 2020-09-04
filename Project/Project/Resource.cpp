#include "Resource.h"

bool Resource::load(Type resoureType, const std::string& resourceName, const std::string& path)
{
    type = resoureType;
    name = resourceName;

    switch (type)
    {
    case Resource::Type::TEXTURE:
        value = sf::Texture();
        return std::get<sf::Texture>(value).loadFromFile(path);

    case Resource::Type::SOUND:
        value = sf::SoundBuffer();
        return std::get<sf::SoundBuffer>(value).loadFromFile(path);

    case Resource::Type::MUSIC:
        value = new sf::Music;
        return std::get<sf::Music*>(value)->openFromFile(path);

    case Resource::Type::FONT:
        value = sf::Font();
        return std::get<sf::Font>(value).loadFromFile(path);

    case Resource::Type::IMAGE:
        value = sf::Image();
        return std::get<sf::Image>(value).loadFromFile(path);

    default:
        return false;
    }
}

std::unordered_map<std::string, Resource> g_resources;
#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

struct Resource
{
    enum class Type
    {
        IMAGE,
        TEXTURE,
        SOUND,
        MUSIC,
        FONT
    };

    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) = delete;
    Resource& operator=(Resource&&) = delete;

    bool load(Type resoureType, const std::string& resourceName, const std::string& path);

    Type getType() const { return type; }

    std::string getName() const { return name; }

    template <Type type>
    auto& get()
    {
        if constexpr (type == Type::IMAGE) return std::get<sf::Image>(value);
        if constexpr (type == Type::TEXTURE) return std::get<sf::Texture>(value);
        if constexpr (type == Type::SOUND) return std::get<sf::SoundBuffer>(value);
        if constexpr (type == Type::MUSIC) return *std::get<sf::Music*>(value);
        if constexpr (type == Type::FONT)  return std::get<sf::Font>(value);
    }

    template <Type type>
    const auto& get() const { return get<type>(); }

private:
    Type type;
    std::string name;
    std::variant<sf::Texture, sf::SoundBuffer, sf::Music*, sf::Font, sf::Image> value;
};

extern std::unordered_map<std::string, Resource> g_resources;
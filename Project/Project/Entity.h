#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <array>
#include <box2d/box2d.h>

struct Scene;
struct Entity
{
    enum class ComponentType
    {
        BODY,
        SHAPE,
        SPRITE,
        ANIMATION,

        COUNT
    };

    Entity(Scene& scene);

    void AddRectangleShape(float width, float height, float xPos = 0, float yPos = 0);
    sf::RectangleShape& GetRectangleShape();

    void AddRectangleBody(float width, float height, float xPos = 0, float yPos = 0, b2BodyType type = b2_staticBody);
    b2Body& GetRectangleBody();

    void AddSprite(float width, float height, float xPos = 0, float yPos = 0, const std::string& texture = "");
    sf::Sprite& GetSprite();

    std::array <int, static_cast<std::size_t>(ComponentType::COUNT)> components;
    Scene& parentScene;
    bool cameraFocus = false;
    std::string name;

private:
    int GetComponentIndex(ComponentType type);
    void SetComonentIndex(ComponentType type, int index);

};
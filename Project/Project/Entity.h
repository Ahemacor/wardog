#pragma once

#include "Animation.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <array>
#include <variant>
#include <optional>
#include <box2d/box2d.h>

struct Entity_
{
    std::string name;
    sf::Transformable transformation;
    b2Vec2 velocity;

    struct Component
    {
        enum class Type
        {
            BODY,
            SHAPE,
            SPRITE,
            ANIMATION,
            CAMERA
        };

        Component() = default;
        Component(const Component&) = default;
        Component& operator=(const Component&) = default;

        Type type;
        std::variant<b2Body*, sf::RectangleShape, sf::Sprite, Animation, sf::View> var;
        sf::Transformable transformation;
    };

    Component* getComponent(Component::Type componentType);

    std::vector<Component> components;
};
#pragma once

#include "Animation.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <array>
#include <variant>
#include <optional>
#include <box2d/box2d.h>
#include <unordered_map>
#include <functional>

struct Entity_;
using Action = std::function<void(Entity_&, bool)>;
using ActionList = std::vector<Action>;
using ControlActions = std::map<sf::Keyboard::Key, ActionList>;

struct Component : public sf::Transformable
{
    enum class Type
    {
        BODY,
        SHAPE,
        SPRITE,
        ANIMATION,
        CAMERA,
        CONTROLLER
    };

    Component() = default;
    Component(const Component&) = default;
    Component& operator=(const Component&) = default;

    Type type;
    std::variant<b2Body*, sf::RectangleShape, sf::Sprite, Animation, sf::View, ControlActions> var;
};

struct Entity_ : public sf::Drawable, public sf::Transformable
{
    std::string name;
    b2Vec2 velocity;

    void update(const sf::Time& elapsedTime);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Component* getComponent(Component::Type componentType);

    std::vector<Component> components;
};
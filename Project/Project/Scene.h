#pragma once

#include "Entity.h"
#include <box2d/box2d.h>
#include <vector>
#include "UiManager.h"
#include <string>
#include <unordered_map>

struct Entity;

struct Scene : public sf::Drawable, public sf::Transformable
{
    Entity* getEntity(const std::string& entityName);

    void update(const sf::Time& elapsedTime);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void setCamera(const sf::Transform& transform, const sf::View& view);

    void clear();

    std::vector<Entity> sceneGraph;

    sf::View view;
    sf::FloatRect viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
    sf::Transform cameraTransform;

    std::vector<std::string> playlist;

    b2World world = b2Vec2(0.0f, 0.0f);

    std::stack<Menu> menuStack;
    std::unordered_map<std::string, Menu> allMenu;
};

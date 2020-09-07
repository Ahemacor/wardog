#pragma once

#include "Entity.h"
#include <box2d/box2d.h>
#include <vector>

struct Entity;

struct Scene : public sf::Drawable, public sf::Transformable
{
    Entity_* getEntity(const std::string& entityName);

    void update(const sf::Time& elapsedTime);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::vector<Entity_> sceneGraph;

    sf::View view;
    sf::FloatRect viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
    sf::Transform cameraTransform;

    std::vector<std::string> playlist;

    b2World world = b2Vec2(0.0f, 0.0f);
};

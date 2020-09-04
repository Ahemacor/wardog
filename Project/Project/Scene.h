#pragma once
#include <box2d/box2d.h>
#include <vector>
#include "Animation.h"

struct Entity;

struct Scene
{
    Scene();

    std::size_t AddEntity(Entity&& entity);

    std::size_t CreateEntity();

    Entity& GetEntityRef(std::size_t entityId);

    Entity& GetEntityRefByName(const std::string& entityName);

    std::vector<Entity> entities;
    std::vector<b2Body*> bodies;
    std::vector<sf::RectangleShape> shapes;
    std::vector<sf::Sprite> sprites;
    std::vector<Animation> animations;

    b2World world;
};

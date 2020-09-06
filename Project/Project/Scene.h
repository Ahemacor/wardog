#pragma once
#include <box2d/box2d.h>
#include <vector>
#include "Animation.h"
#include "Entity.h"

struct Entity;

struct Scene
{
    Scene();

    Entity_& getEntity(const std::string& entityName);

    std::vector<Entity_> sceneGraph;

    b2World world;
};

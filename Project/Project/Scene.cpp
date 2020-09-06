#include "Scene.h"
#include "CommonDefinitions.h"

Scene::Scene() : world(b2Vec2(0.0f, 0.0f)) {}

Entity_& Scene::getEntity(const std::string& entityName)
{
    return *std::find_if(sceneGraph.begin(),
                         sceneGraph.end(),
                         [entityName](auto& entity) {return entity.name == entityName; });
}
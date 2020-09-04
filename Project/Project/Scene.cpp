#include "Scene.h"
#include "CommonDefinitions.h"

Scene::Scene() : world(b2Vec2(0.0f, 0.0f)) {}

std::size_t Scene::AddEntity(Entity&& entity)
{
    const std::size_t entityId = entities.size();
    entities.push_back(std::move(entity));
    return entityId;
}

std::size_t Scene::CreateEntity()
{
    const std::size_t entityId = entities.size();
    entities.push_back(Entity(*this));
    return entityId;
}

Entity& Scene::GetEntityRef(std::size_t entityId)
{
    return entities[entityId];
}

Entity& Scene::GetEntityRefByName(const std::string& entityName)
{
    Entity& pEntity = *std::find_if(entities.begin(),
        entities.end(),
        [entityName](auto& entity) {return entity.name == entityName; });
    return pEntity;
}
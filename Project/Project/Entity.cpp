#include "Entity.h"
#include "CommonDefinitions.h"

Entity_::Component* Entity_::getComponent(Entity_::Component::Type componentType)
{
    for (auto& component : components)
    {
        if (component.type == componentType)
        {
            return &component;
        }
    }
    return nullptr;
}
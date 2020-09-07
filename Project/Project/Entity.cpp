#include "Entity.h"
#include "CommonDefinitions.h"

Component* Entity_::getComponent(Component::Type componentType)
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

void Entity_::update(const sf::Time& elapsedTime)
{
    for (auto& component : components)
    {
        switch (component.type)
        {
        case Component::Type::BODY:
        {
            b2Body* pBody = std::get<b2Body*>(component.var);
            const b2Vec2 bodyPosition = pBody->GetPosition();
            setPosition({ (float)meterToPixel(bodyPosition.x), (float)meterToPixel(bodyPosition.y) });
            setRotation(radianToDegree(pBody->GetAngle()));
            velocity = pBody->GetLinearVelocity();
        }
        break;

        case Component::Type::SHAPE:
        {
            sf::RectangleShape& shape = std::get<sf::RectangleShape>(component.var);
        }
        break;

        case Component::Type::SPRITE:
        {
            sf::Sprite& sprite = std::get<sf::Sprite>(component.var);
        }
        break;

        case Component::Type::ANIMATION:
        {
            Animation& animation = std::get<Animation>(component.var);
            animation.update(velocity);
        }
        break;
        case Component::Type::CAMERA:
        {
            GAME_INSTANCE.scene.cameraTransform = getTransform();
            GAME_INSTANCE.scene.view = std::get<sf::View>(component.var);
        }
        break;
        case Component::Type::CONTROLLER:
        {
            ControlActions& controls = std::get<ControlActions>(component.var);
            for (auto& [key, actions] : controls)
            {
                for (auto& action : actions) action(*this, sf::Keyboard::isKeyPressed(key));
            }
        }
        break;

        }
    }
}

void Entity_::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (auto& component : components)
    {
        sf::RenderStates renderState;
        renderState.transform *= GAME_INSTANCE.scene.cameraTransform.getInverse();
        renderState.transform *= getTransform();

        switch (component.type)
        {
            renderState.transform *= component.getTransform();
        case Component::Type::BODY:
        {
            b2Body* pBody = std::get<b2Body*>(component.var);
            break;
        }

        case Component::Type::SHAPE:
        {
            const sf::RectangleShape& shape = std::get<sf::RectangleShape>(component.var);
            target.draw(shape, renderState);
            break;
        }

        case Component::Type::SPRITE:
        {
            const sf::Sprite& sprite = std::get<sf::Sprite>(component.var);
            target.draw(sprite, renderState);
            break;
        }

        case Component::Type::ANIMATION:
        {
            const Animation& animation = std::get<Animation>(component.var);
            target.draw(animation, renderState);
            break;
        }

        }
    }
}

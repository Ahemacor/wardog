#include "Entity.h"
#include "CommonDefinitions.h"

int Entity::GetComponentIndex(ComponentType type)
{
    return components[static_cast<std::size_t>(type)];
}

void Entity::SetComonentIndex(ComponentType type, int index)
{
    components[static_cast<std::size_t>(type)] = index;
}

Entity::Entity(Scene& scene) : parentScene(scene)
{
    for (auto& component : components) component = -1;
}

void Entity::AddRectangleShape(float width, float height, float xPos, float yPos)
{
    sf::RectangleShape newRect;
    newRect.setSize({ width, height });
    newRect.setOrigin(width / 2, height / 2);
    newRect.setPosition(xPos, yPos);

    const int bindingIndex = GetComponentIndex(ComponentType::SHAPE);
    if (bindingIndex < 0)
    {
        SetComonentIndex(ComponentType::SHAPE, parentScene.shapes.size());
        parentScene.shapes.push_back(std::move(newRect));
    }
    else
    {
        parentScene.shapes[bindingIndex] = newRect;
    }
}

sf::RectangleShape& Entity::GetRectangleShape()
{
    const int bindingIndex = GetComponentIndex(ComponentType::SHAPE);
    return parentScene.shapes[bindingIndex];
}

void Entity::AddRectangleBody(float width, float height, float xPos, float yPos, b2BodyType type)
{
    b2BodyDef bodyDef;
    bodyDef.type = type;
    bodyDef.position.Set(xPos, yPos);
    bodyDef.angularDamping = 15.0f;
    bodyDef.linearDamping = 10.0f;
    b2Body* body = parentScene.world.CreateBody(&bodyDef);

    b2PolygonShape boxShape;
    boxShape.SetAsBox(width / 2, height / 2);

    b2FixtureDef fixture;
    fixture.shape = &boxShape;
    fixture.density = 1.0f;
    fixture.friction = 0.8f;
    body->CreateFixture(&fixture);

    body->SetFixedRotation(true);

    const int bindingIndex = GetComponentIndex(ComponentType::BODY);
    if (bindingIndex < 0)
    {
        SetComonentIndex(ComponentType::BODY, parentScene.bodies.size());
        parentScene.bodies.push_back(body);
    }
    else
    {
        parentScene.bodies[bindingIndex] = body;
    }
}

b2Body& Entity::GetRectangleBody()
{
    const int bindingIndex = GetComponentIndex(ComponentType::BODY);
    return *parentScene.bodies[bindingIndex];
}

void Entity::AddSprite(float width, float height, float xPos, float yPos, const std::string& texture)
{
    sf::Sprite sprite;
    //sprite.setSize({ width, height });
    sf::IntRect intRect({ (int)xPos, (int)yPos }, { (int)width, (int)height });
    sprite.setTextureRect(intRect);
    sprite.setOrigin(width / 2, height / 2);
    sprite.setPosition(xPos, yPos);
    sprite.setTexture(TEXTURE(texture));

    const int bindingIndex = GetComponentIndex(ComponentType::SPRITE);
    if (bindingIndex < 0)
    {
        SetComonentIndex(ComponentType::SPRITE, parentScene.sprites.size());
        parentScene.sprites.push_back(std::move(sprite));
    }
    else
    {
        parentScene.sprites[bindingIndex] = sprite;
    }
}
sf::Sprite& Entity::GetSprite()
{
    const int bindingIndex = GetComponentIndex(ComponentType::SPRITE);
    return parentScene.sprites[bindingIndex];
}
#include "Scene.h"
#include "CommonDefinitions.h"
#include <variant>
#include <algorithm>
#include <chrono>
#include <random>

Entity* Scene::getEntity(const std::string& entityName)
{
    auto findIt = std::find_if(sceneGraph.begin(),
                               sceneGraph.end(),
                               [entityName](auto& entity) {return entity.name == entityName; });
    return (findIt != sceneGraph.end()) ? findIt._Ptr : nullptr;
}

void Scene::update(const sf::Time& elapsedTime)
{
    view = GAME_INSTANCE.window.getDefaultView();

    const int32 velocityIterations = 50;
    const int32 positionIterations = 50;
    world.Step(elapsedTime.asSeconds(), velocityIterations, positionIterations);

    for (auto& entity : sceneGraph)
    {
        entity.update(elapsedTime);
    }

    if (!AudioSystem::getInstance().isMusicPlaying() && playlist.size() > 0)
    {
        const std::string currentMusicName = AudioSystem::getInstance().getCurrentMusic();
        if (currentMusicName.empty())
        {
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(playlist.begin(), playlist.end(), std::default_random_engine(seed));
        }

        int musicNameIdx = 0;
        for (const auto& musName : playlist)
        {
            ++musicNameIdx;
            if (musName == currentMusicName)
            {
                break;
            }
        }
        musicNameIdx = musicNameIdx % playlist.size();
        const std::string& nextMusic = playlist[musicNameIdx];
        PLAY_MUSIC(playlist[musicNameIdx]);
        LOG_INFO(std::string("play: ") + nextMusic);
    }
}

void Scene::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::RenderStates renderState = states;
    renderState.transform *= getTransform();
    renderState.transform *= cameraTransform.getInverse();
    sf::View prevView = target.getView();
    sf::View sceneView = view;
    sceneView.setViewport(viewport);
    target.setView(sceneView);

    for (auto& entity : sceneGraph)
    {
        target.draw(entity, renderState);
    }

    target.setView(prevView);
}

void Scene::setCamera(const sf::Transform& transform, const sf::View& view)
{
    cameraTransform = transform;
    this->view = view;
}

void Scene::clear()
{
    sceneGraph.clear();

    view = GAME_INSTANCE.window.getDefaultView();
    viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
    cameraTransform = sf::Transform::Identity;

    playlist.clear();
    AudioSystem::getInstance().stopMusic();

    b2Body* body = world.GetBodyList();
    while (body != nullptr)
    {
        b2Body* nextBody = body->GetNext();
        world.DestroyBody(body);
        body = nextBody;
    }


    while (!menuStack.empty()) menuStack.pop();

    allMenu.clear();

    //g_resources.clear();
}
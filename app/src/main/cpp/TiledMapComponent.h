#pragma once

#include "Component.h"
#include "TiledMap.h"

class TiledMapComponent : public Component
{
    TiledMap* map = nullptr;
public:
    TiledMapComponent(String levelName, TiledMap::TiledMapOptions* tiledMapOptions, Actor* owner)
        : Component(utils::getGUID(), owner, Component::instantiateFunc<TiledMapComponent>())
    {
        Window& window = tiledMapOptions->window;
        map = window.getAssetManager()->getOrAddRes<TiledMap>(levelName, (void*)tiledMapOptions);
    }
    void updateAndRender(float dt) { map->draw(); };
};

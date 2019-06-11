#pragma once

#include "Component.h"
#include "TiledMap.h"
#include "Globals.h"

class TiledMapComponent : public Component
{
    TiledMap* map = nullptr;
public:
    TiledMapComponent(String levelName)
        : Component(utils::getGUID(), Component::instantiateFunc<TiledMapComponent>())
    {
        map = Globals::window->getAssetManager()->getOrAddRes<TiledMap>(levelName);
    }
    void updateAndRender(float dt) { map->draw(); };
};

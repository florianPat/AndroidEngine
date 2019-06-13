#include "GameObjectManager.h"
#include "Utils.h"

GameObjectManager::GameObjectManager() : actors(), destroyActorQueue()
{
    //NOTE: This is the RenderOnlyActorThing
    addActor();
}

const Actor* GameObjectManager::addActor()
{
	actors.push_back(Actor(actors.size()));

	return &actors.back();
}

void GameObjectManager::destroyActor(unsigned int actorId)
{
	destroyActorQueue.push_back(actorId);
}

void GameObjectManager::updateAndDrawActors(float dt)
{
    //NOTE: ++ because the first one is reserved for the only draw components
	for (auto it = (++actors.begin()); it != actors.end(); ++it)
		it->update(dt);

	for(int i = 0; i < arrayCount(layers); ++i)
	{
		auto& layer = layers[i];

		for(auto it = layer.begin(); it != layer.end(); ++it)
		{
			Actor& actor = actors[it->first];
			actor.getComponent<Component>(it->second)->render();
		}
	}

	destroyActors();
}

void GameObjectManager::destroyActors()
{
	if (!destroyActorQueue.empty())
	{
		for (auto it = destroyActorQueue.begin(); it != destroyActorQueue.end(); ++it)
		{
			actors.erasePop_back(*it);
			actors[*it].id = *it;
		}
		destroyActorQueue.clear();
	}
}

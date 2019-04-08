#include "GameObjectManager.h"
#include "Utils.h"

GameObjectManager::GameObjectManager() : actors(), sortedActors(), destroyActorQueue()
{
}

Actor* GameObjectManager::addActor()
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
	for (auto it = actors.begin(); it != actors.end(); ++it)
		it->updateAndDraw(dt);

	destroyActors();
}

void GameObjectManager::sortActors()
{
	for (auto it = actors.begin(); it != actors.end(); ++it)
	{
		it->sort(sortedActors);
	}
}

void GameObjectManager::destroyActors()
{
	if (!destroyActorQueue.empty())
	{
		for (auto it = destroyActorQueue.begin(); it != destroyActorQueue.end(); ++it)
		{
			Actor& actorIt = actors.at(*it);
			actorIt.clearComponents();
			actors.erasePop_back(*it);
		}
		destroyActorQueue.clear();
	}
}

unsigned int GameObjectManager::getActorId(unsigned long long id)
{
	return (id >> 32);
}

unsigned int GameObjectManager::getComponentId(unsigned long long id)
{
	return (id & 0xffffffff);
}

#pragma once

#include <map>
#include "Actor.h"

class GameObjectManager
{
	Vector<Actor> actors;
	std::multimap<gomSort::SortKey, unsigned long long, gomSort::SortCompare> sortedActors;
	Vector<int> destroyActorQueue;
public:
	GameObjectManager();
	Actor* addActor();
	void destroyActor(unsigned int actorId);
	void updateAndDrawActors(float dt);
	void sortActors();
private:
	void destroyActors();
	unsigned int getActorId(unsigned long long id);
	unsigned int getComponentId(unsigned long long id);
};
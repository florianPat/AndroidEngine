#pragma once

#include "Actor.h"

class GameObjectManager
{
	Vector<Actor> actors;
	Vector<int> destroyActorQueue;
public:
	GameObjectManager();
	Actor* addActor();
	void destroyActor(unsigned int actorId);
	void updateAndDrawActors(float dt);
private:
	void destroyActors();
	unsigned int getActorId(unsigned long long id);
	unsigned int getComponentId(unsigned long long id);
};
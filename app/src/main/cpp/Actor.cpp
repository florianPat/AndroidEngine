#include "Actor.h"
#include "GameObjectManager.h"

Actor::Actor(unsigned int id)
	:	id(id), components()
{
}

void Actor::addComponent(std::unique_ptr<Component> component)
{
	components.push_back(std::move(component));
}

void Actor::removeComponent(unsigned int componentId)
{
	for(auto it = components.begin(); it != components.end(); ++it)
	{
		if((*it)->getId() == componentId)
		{
			components.erasePop_back(it.getIndex());
			return;
		}
	}
}

void Actor::clearComponents()
{
	components.clear();
}

void Actor::updateAndDraw(float dt)
{
	for (auto it = components.begin(); it != components.end(); ++it)
		(*it)->updateAndDraw(dt);
}

unsigned long long Actor::GetActorComponentId(unsigned int componentId)
{
	return ((unsigned long long)id << 32llu) | (unsigned long long)componentId;
}
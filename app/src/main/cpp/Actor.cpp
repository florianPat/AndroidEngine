#include "Actor.h"
#include "GameObjectManager.h"

Actor::Actor(unsigned int id) : id(id), components()
{
}

void Actor::addComponent(std::unique_ptr<Component> component)
{
	if (components.find(component->getId()) == components.end())
	{
		components.emplace(component->getId(), std::move(component));
	}
}

void Actor::removeComponent(unsigned int componentId)
{
	auto it = components.find(componentId);

	if (it != components.end())
	{
		components.erase(it);
	}
}

void Actor::clearComponents()
{
	components.clear();
}

void Actor::updateAndDraw(float dt)
{
	for (auto it = components.begin(); it != components.end(); ++it)
		it->second->updateAndDraw(dt);
}

void Actor::sort(std::multimap<gomSort::SortKey, unsigned long long, gomSort::SortCompare>& sortedActors)
{
	for (auto it = components.begin(); it != components.end(); ++it)
	{
		gomSort::SortKey sortKey = it->second->sort();
		unsigned long long actorComponentId = GetActorComponentId(it->second->getId());
		sortedActors.emplace(sortKey, actorComponentId);
	}
}

unsigned long long Actor::GetActorComponentId(unsigned int componentId)
{
	return ((unsigned long long)id << 32llu) | (unsigned long long)componentId;
}
#include "Actor.h"
#include "GameObjectManager.h"

Actor::Actor(unsigned int id)
	:	id(id), components()
{
}

void Actor::update(float dt)
{
	for (auto it = components.begin(); it != components.end();)
	{
		uint compSize = *((uint*) it);
		it += sizeof(uint);

		((Component*)(it))->update(dt, this);

		it += compSize;
	}
}

int Actor::getComponentIndex(uint componentId) const
{
	for(auto it = components.begin(); it != components.end();)
	{
		uint compSize = *((uint*) it);
		it += sizeof(uint);

		if(((Component*)(it))->getId() == componentId)
			return (int)(it - components.begin());

		it += compSize;
	}

	return -1;
}

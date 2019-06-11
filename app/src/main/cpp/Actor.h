#pragma once

#include "Component.h"
#include <memory>
#include "Window.h"
#include "Physics.h"

class Actor
{
	const unsigned int id;
	//TODO: Think about if I can refactor Actor do not use an unique_ptr here! But!
	//NOTE: The pointers do not get invalidated in shrink/expand because I just store a pp
	Vector<std::unique_ptr<Component>> components;
private:
	unsigned long long GetActorComponentId(unsigned int componentId);
public:
	Actor(unsigned int id);
	void addComponent(std::unique_ptr<Component> component);
	void clearComponents();
	//NOTE: These methods not take O(n) (because of the linear search). Do not call these functions
	// often.
	//TODO: Optimization?
	void removeComponent(unsigned int componentId);
	template <typename T> T* getComponent(unsigned int componentId);

	int getId() const { return id; };
	void updateAndDraw(float dt);
};

template<typename T>
inline T* Actor::getComponent(unsigned int componentId)
{
	for(auto it = components.begin(); it != components.end(); ++it)
	{
		if((*it)->getId() == componentId)
		{
			Component* componentPtr = it->get();

			//NOTE: Only used to really verify that it is ok what I am doing. RTTI should be switched off in release mode
			assert(typeid((*componentPtr)) == typeid(T));
			assert(dynamic_cast<T*>(componentPtr) != nullptr);

			return (T*) componentPtr;
		}
	}

	return nullptr;
}

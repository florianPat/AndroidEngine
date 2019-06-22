#pragma once

#include "Component.h"
#include "Window.h"
#include "Physics.h"
#include "VariableComponentVector.h"

class Actor
{
    //Needs to happen because it has to change the id if an Actor gets deleted!
    friend class GameObjectManager;

	unsigned int id;
	VariableComponentVector components;
public:
	Actor(unsigned int id, size_t componentsSize);
	template <typename T, typename... Args>
	const T* addComponent(Args&&... args);
	//NOTE: This method takes O(n) (because of the linear search). Do not call this function often.
	int getComponentIndex(uint componentId) const;

	template <typename T> T* getComponent(int componentIndex);

	int getId() const { return id; };
	void update(float dt);
};

template<typename T>
inline T* Actor::getComponent(int componentIndex)
{
	assert(componentIndex < components.getOffsetToEnd());
	Component* componentPtr = (Component*) (components.begin() + componentIndex);

	//NOTE: Only used to really verify that it is ok what I am doing. RTTI should be switched off in release mode
	//assert(typeid((*componentPtr)) == typeid(T));
	assert(dynamic_cast<T*>(componentPtr) != nullptr);

	return (T*) componentPtr;
}

template<typename T, typename... Args>
inline const T* Actor::addComponent(Args&&... args)
{
	size_t lastOffsetToEnd = components.getOffsetToEnd();

	components.push_back<T>(std::forward<Args>(args)...);

	return (T*)(components.begin() + lastOffsetToEnd + 4);
}
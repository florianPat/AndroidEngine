#pragma once

#include <unordered_map>
#include "EventData.h"
#include <functional>
#include <memory>
#include "Vector.h"

//TODO: Replace std::function
typedef std::pair<unsigned int, std::function<void(EventData*)>> DelegateFunction;

class EventManager
{
	//TODO: I could really refactor this to use a Vector, or?
	std::unordered_map<unsigned int, Vector<DelegateFunction>> eventListenerMap;
	Vector<std::pair<unsigned int, DelegateFunction>> eventDeleterMap;
public:
	EventManager();
	bool addListener(unsigned int eventType, DelegateFunction& delegateFunction);
	void removeListener(unsigned int eventType, DelegateFunction& delegateFunction);
	bool TriggerEvent(std::unique_ptr<EventData> eventData);
	void removeListeners();
};

namespace utils
{
	DelegateFunction getDelegateFromFunction(std::function<void(EventData*)>& function);
}
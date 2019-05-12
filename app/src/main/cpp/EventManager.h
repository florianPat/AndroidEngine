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
	Vector<Vector<DelegateFunction>> eventListenerMap;
	Vector<std::pair<unsigned int, DelegateFunction>> eventDeleterMap;
private:
	uint counter = 0;
public:
	DelegateFunction getDelegateFromFunction(std::function<void(EventData*)>& function);
public:
	EventManager();
	bool addListener(unsigned int eventType, DelegateFunction& delegateFunction);
	void removeListener(unsigned int eventType, DelegateFunction& delegateFunction);
	void TriggerEvent(std::unique_ptr<EventData> eventData);
	void removeListeners();
};
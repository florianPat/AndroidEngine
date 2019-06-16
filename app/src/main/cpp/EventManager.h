#pragma once

#include "EventData.h"
#include <functional>
#include <memory>
#include "Vector.h"

//TODO: Replace std::function
typedef std::pair<unsigned int, std::function<void(EventData*)>> DelegateFunction;

class EventManager
{
    struct EventListenerMapEntry
    {
        int& eventType;
        Vector<DelegateFunction> delegateFunctions;
    };

	Vector<EventListenerMapEntry> eventListenerMap;
	Vector<std::pair<int, DelegateFunction>> eventDeleterMap;
private:
	uint counter = 0;
public:
	DelegateFunction getDelegateFromFunction(std::function<void(EventData*)>&& function);
public:
	EventManager();
	~EventManager();
	bool addListener(int& eventType, const DelegateFunction& delegateFunction);
	void removeListener(int eventType, const DelegateFunction& delegateFunction);
	void TriggerEvent(EventData* eventData);
	void removeListeners();
};
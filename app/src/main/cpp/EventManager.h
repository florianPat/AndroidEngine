#pragma once

#include "EventData.h"
#include "Vector.h"
#include "Delegate.h"
#include "Globals.h"

typedef std::pair<unsigned int, Delegate<void(EventData*)>> DelegateFunction;

class EventManager
{
    struct EventListenerMapEntry
    {
        int& eventType;
        Vector<DelegateFunction> delegateFunctions;
    };
public:
	struct DelegateFunctionRef
	{
		const int& eventType;
		const uint delegateId;
	};
private:
	Vector<EventListenerMapEntry> eventListenerMap;
	Vector<DelegateFunctionRef> eventDeleterMap;
	uint counter = 0;
private:
    DelegateFunction getDelegateFromFunction(Delegate<void(EventData*)>&& function);
public:
	EventManager();
	~EventManager();
	DelegateFunctionRef addListener(int& eventType, Delegate<void(EventData*)>&& function);
	void removeListener(const DelegateFunctionRef& delegateFunctionRef);
	void TriggerEvent(EventData* eventData);
	void removeListeners();
};
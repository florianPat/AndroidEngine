#pragma once

#include "EventData.h"
#include "Vector.h"
#include "Delegate.h"
#include "Globals.h"

typedef std::pair<uint32_t, Delegate<void(EventData*)>> DelegateFunction;

class EventManager
{
    struct EventListenerMapEntry
    {
        int32_t& eventType;
        Vector<DelegateFunction> delegateFunctions;
    };
public:
	struct DelegateFunctionRef
	{
		const int32_t& eventType;
		const uint32_t delegateId;
	};
private:
	Vector<EventListenerMapEntry> eventListenerMap;
	Vector<DelegateFunctionRef> eventDeleterMap;
	uint32_t counter = 0;
private:
    DelegateFunction getDelegateFromFunction(Delegate<void(EventData*)>&& function);
public:
	EventManager();
	~EventManager();
	DelegateFunctionRef addListener(int32_t& eventType, Delegate<void(EventData*)>&& function);
	void removeListener(const DelegateFunctionRef& delegateFunctionRef);
	void TriggerEvent(EventData* eventData);
	void removeListeners();
};
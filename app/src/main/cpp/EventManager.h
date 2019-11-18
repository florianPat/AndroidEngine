#pragma once

#include "EventData.h"
#include "Vector.h"
#include "Delegate.h"
#include "Globals.h"
#include "VariableVector.h"
#include "NativeThreadQueue.h"

class EventManager
{
    struct EventListenerMapEntry
    {
    	int32_t eventType;
		Delegate<void(EventData*)> function;
    };
    struct EventTypeAndHolderVector
	{
    	int32_t& eventType;
		VariableVector<EventData> eventDataHolder;
	};
public:
	struct DelegateFunctionRef
	{
		const uint32_t threadTypeId;
		const uint32_t delegateId;
	};
private:
	Vector<Vector<EventListenerMapEntry>> eventListenerMap;
	Vector<EventTypeAndHolderVector> eventTypeVector;
	Vector<DelegateFunctionRef> eventDeleterMap;
	NativeThreadQueue& nativeThreadQueue;
	volatile uint32_t mutex = 0;
private:
	void triggerEventDelegate(uint32_t specificArg, float broadArg);
	void clearTriggerEventsDelegate(uint32_t specificArg, float broadArg);
public:
	EventManager();
	~EventManager();
	DelegateFunctionRef addListener(int32_t& eventType, Delegate<void(EventData*)>&& function);
	DelegateFunctionRef addListenerForSameThreadType(int32_t& eventType, Delegate<void(EventData*)>&& function, uint32_t threadTypeId);
	template <typename T, typename... Args>
	void TriggerEvent(Args&&... args);
	void addClearTriggerEventsJob();
	void addListenerEventsJobs();
};

template <typename T, typename... Args>
inline void EventManager::TriggerEvent(Args&&... args)
{
	if(T::eventId != -1)
	{
		assert((uint32_t)T::eventId < eventTypeVector.size());
		VariableVector<EventData>& eventDataHolder = eventTypeVector[T::eventId].eventDataHolder;

		bool written;
		do {
			written = __sync_bool_compare_and_swap(&mutex, 0, 1);
		} while(!written);

		eventDataHolder.push_back<T>(std::forward<Args>(args)...);

		__sync_synchronize();

        mutex = 0;
	}
	else
	{
		utils::log("Event is not registered with any functions yet!");
	}
}
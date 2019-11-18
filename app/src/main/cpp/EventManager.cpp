#include "EventManager.h"
#include "Utils.h"
#include "Window.h"

EventManager::EventManager() : eventListenerMap(), nativeThreadQueue(Globals::window->getNativeThreadQueue())
{
}

EventManager::DelegateFunctionRef EventManager::addListener(int32_t& eventType, Delegate<void(EventData*)>&& function)
{
    eventListenerMap.push_back(Vector<EventListenerMapEntry>());
    return addListenerForSameThreadType(eventType, std::move(function), eventListenerMap.size() - 1);
}

EventManager::DelegateFunctionRef EventManager::addListenerForSameThreadType(int32_t& eventType,
		Delegate<void(EventData*)>&& function, uint32_t threadTypeId)
{
    assert(!nativeThreadQueue.getStartedFlushing());

    if (eventType == -1)
    {
        eventType = eventTypeVector.size();
        eventTypeVector.push_back(EventTypeAndHolderVector{ eventType, VariableVector<EventData>(sizeof(EventData) * 10) });
    }
    Vector<EventListenerMapEntry>* eventListenerList = &eventListenerMap[threadTypeId];

    eventListenerList->push_back(EventListenerMapEntry{ eventType, function });

	return DelegateFunctionRef{ threadTypeId, eventListenerList->size() - 1 };
}

void EventManager::addClearTriggerEventsJob()
{
    nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float), EventManager,
            &EventManager::clearTriggerEventsDelegate), 0);
}

void EventManager::addListenerEventsJobs()
{
    for(uint32_t i = 0; i < eventListenerMap.size(); ++i)
    {
        nativeThreadQueue.addEntry(GET_DELEGATE_WITH_PARAM_FORM(void(uint32_t, float), EventManager,
                                                                &EventManager::triggerEventDelegate), i);
    }
}

void EventManager::triggerEventDelegate(uint32_t specificArg, float broadArg)
{
    Vector<EventListenerMapEntry>* eventListenerList = &eventListenerMap[specificArg];

    for(auto it = eventListenerList->begin(); it != eventListenerList->end(); ++it)
    {
        VariableVector<EventData>& dataHolderOffset = eventTypeVector[it->eventType].eventDataHolder;
        for(auto offsetIt = dataHolderOffset.begin(); offsetIt != dataHolderOffset.end();)
        {
            uint32_t eventDataSize = *((uint32_t*)offsetIt);
            offsetIt += sizeof(uint32_t);

            it->function((EventData*)(offsetIt));

            offsetIt += eventDataSize;
        }
    }
}

void EventManager::clearTriggerEventsDelegate(uint32_t specificArg, float broadArg)
{
    for(auto it = eventTypeVector.begin(); it != eventTypeVector.end(); ++it)
    {
        it->eventDataHolder.clear();
    }
}

EventManager::~EventManager()
{
    for(auto it = eventTypeVector.begin(); it != eventTypeVector.end(); ++it)
    {
        it->eventType = -1;
    }

    eventListenerMap.~Vector();
    eventDeleterMap.~Vector();
    eventTypeVector.~Vector();
}

//void EventManager::removeListener(const DelegateFunctionRef& delegateFunctionRef)
//{
//	eventDeleterMap.push_back(delegateFunctionRef);
//}
//
//void EventManager::removeListeners()
//{
//	if (!eventDeleterMap.empty())
//	{
//		for (auto it = eventDeleterMap.begin(); it != eventDeleterMap.end(); ++it)
//		{
//			int32_t eventType = it->eventType;
//			uint32_t delegateFunctionId = it->delegateId;
//
//			assert(eventType < eventListenerMap.size());
//			auto foundTuple = eventListenerMap.at(eventType);
//			auto findIt = foundTuple.delegateFunctions;
//			//TODO: Optimize this!
//			for (auto it = findIt.begin(); it != findIt.end(); ++it)
//			{
//				if (delegateFunctionId == it->index)
//				{
//					findIt.erasePop_back(it);
//					break;
//				}
//			}
//			if (findIt.empty())
//			{
//			    int32_t eventType = foundTuple.eventType;
//                foundTuple.eventType = -1;
//				eventListenerMap.erasePop_back(eventType);
//				eventListenerMap.at(eventType).eventType = eventType;
//			}
//		}
//		eventDeleterMap.clear();
//	}
//}
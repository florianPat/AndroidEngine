#include "EventManager.h"
#include "Utils.h"

EventManager::EventManager() : eventListenerMap()
{
}

EventManager::DelegateFunctionRef EventManager::addListener(int& eventType, Delegate<void(EventData*)>&& function)
{
    Vector<DelegateFunction>* eventListenerList = nullptr;

	if (eventType == -1)
	{
		eventType = eventListenerMap.size();
		eventListenerMap.push_back(EventListenerMapEntry{eventType, Vector<DelegateFunction>()});
		eventListenerList = &eventListenerMap.back().delegateFunctions;
	}
	else
		eventListenerList = &eventListenerMap.at(eventType).delegateFunctions;

	eventListenerList->push_back(getDelegateFromFunction(std::move(function)));

	return DelegateFunctionRef{ eventType, counter - 1 };
}

void EventManager::removeListener(const DelegateFunctionRef& delegateFunctionRef)
{
	eventDeleterMap.push_back(delegateFunctionRef);
}

void EventManager::TriggerEvent(EventData* eventData)
{
	assert(eventData->getEventId() < eventListenerMap.size());
	auto findIt = eventListenerMap.at(eventData->getEventId()).delegateFunctions;
	for (auto it = findIt.begin(); it != findIt.end(); ++it)
	{
		it->second(eventData);
	}
}

EventManager::~EventManager()
{
	for(auto it = eventListenerMap.begin(); it != eventListenerMap.end(); ++it)
	{
		it->eventType = -1;
	}

	counter = 0;

	eventListenerMap.~Vector();
	eventDeleterMap.~Vector();
}

void EventManager::removeListeners()
{
	if (!eventDeleterMap.empty())
	{
		for (auto it = eventDeleterMap.begin(); it != eventDeleterMap.end(); ++it)
		{
			int eventType = it->eventType;
			uint delegateFunctionId = it->delegateId;

			assert(eventType < eventListenerMap.size());
			auto foundTuple = eventListenerMap.at(eventType);
			auto findIt = foundTuple.delegateFunctions;
			for (auto it = findIt.begin(); it != findIt.end(); ++it)
			{
				if (delegateFunctionId == it->first)
				{
					findIt.erasePop_back(it);
					break;
				}
			}
			if (findIt.empty())
			{
			    int eventType = foundTuple.eventType;
                foundTuple.eventType = -1;
				eventListenerMap.erasePop_back(eventType);
				eventListenerMap.at(eventType).eventType = eventType;
			}
		}
		eventDeleterMap.clear();
	}
}

DelegateFunction EventManager::getDelegateFromFunction(Delegate<void(EventData*)>&& function)
{
	return DelegateFunction(std::pair<unsigned int, Delegate<void(EventData*)>>(counter++, std::move(function)));
}

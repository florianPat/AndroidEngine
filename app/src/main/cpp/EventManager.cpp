#include "EventManager.h"
#include "Utils.h"

EventManager::EventManager() : eventListenerMap()
{
}

bool EventManager::addListener(int& eventType, const DelegateFunction & delegateFunction)
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

#ifdef DEBUG
	for (auto it = eventListenerList->begin(); it != eventListenerList->end();
		++it)
	{
		if (delegateFunction.first == it->first)
		{
			utils::logBreak("Attempting to double - register a delegate");
			return false;
		}
	}
#endif

	eventListenerList->push_back(delegateFunction);
	return true;
}

void EventManager::removeListener(int eventType, const DelegateFunction & delegateFunction)
{
	eventDeleterMap.push_back(std::make_pair(eventType, delegateFunction));
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
			int eventType = it->first;
			DelegateFunction delegateFunction = it->second;

			assert(eventType < eventListenerMap.size());
			auto foundTuple = eventListenerMap.at(eventType);
			auto findIt = foundTuple.delegateFunctions;
			for (auto it = findIt.begin(); it != findIt.end(); ++it)
			{
				if (delegateFunction.first == it->first)
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

DelegateFunction EventManager::getDelegateFromFunction(std::function<void(EventData*)>&& function)
{
	return DelegateFunction(std::pair<unsigned int, std::function<void(EventData*)>>(counter++, std::move(function)));
}

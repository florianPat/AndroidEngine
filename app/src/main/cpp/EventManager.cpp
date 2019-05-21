#include "EventManager.h"
#include "Utils.h"

EventManager::EventManager() : eventListenerMap()
{
}

bool EventManager::addListener(unsigned int eventType, DelegateFunction & delegateFunction)
{
    Vector<DelegateFunction>* eventListenerList = nullptr;

	if (eventType >= eventListenerMap.size())
	{
		eventListenerMap.push_back(Vector<DelegateFunction>());
		eventListenerList = &eventListenerMap.back();
	}
	else
		eventListenerList = &eventListenerMap.at(eventType);

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

void EventManager::removeListener(unsigned int eventType, DelegateFunction & delegateFunction)
{
	eventDeleterMap.push_back(std::make_pair(eventType, delegateFunction));
}

void EventManager::TriggerEvent(std::unique_ptr<EventData> eventData)
{
	assert(eventData->getEventId() < eventListenerMap.size());
	auto findIt = eventListenerMap.at(eventData->getEventId());
	for (auto it = findIt.begin(); it != findIt.end(); ++it)
	{
		it->second(eventData.get());
	}
}

void EventManager::removeListeners()
{
	if (!eventDeleterMap.empty())
	{
		for (auto it = eventDeleterMap.begin(); it != eventDeleterMap.end(); ++it)
		{
			uint eventType = it->first;
			DelegateFunction delegateFunction = it->second;

			assert(eventType < eventListenerMap.size());
			auto findIt = eventListenerMap.at(eventType);
			for (auto it = findIt.begin(); it != findIt.end(); ++it)
			{
				if (delegateFunction.first == it->first)
				{
					findIt.erasePop_back(it);
					break;
				}
			}
			if (findIt.empty())
				eventListenerMap.erase(eventType);
		}
		eventDeleterMap.clear();
	}
}

DelegateFunction EventManager::getDelegateFromFunction(const std::function<void(EventData*)>& function)
{
	return DelegateFunction(std::pair<unsigned int, std::function<void(EventData*)>>(counter++, function));
}

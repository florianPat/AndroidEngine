#pragma once

#include "EventData.h"
#include "Utils.h"

struct EventLevelReload : public EventData
{
	inline static int eventId = -1;
	EventLevelReload() : EventData(eventId) {}
};
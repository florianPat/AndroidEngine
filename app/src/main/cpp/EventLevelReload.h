#pragma once

#include "EventData.h"
#include "Utils.h"

struct EventLevelReload : public EventData
{
	static unsigned int eventId;
	EventLevelReload();
};
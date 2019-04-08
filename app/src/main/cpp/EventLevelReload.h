#pragma once

#include "EventData.h"
#include "Utils.h"

struct EventLevelReload : public EventData
{
	static const unsigned int eventId;
	EventLevelReload();
};
#include "EventLevelReload.h"

unsigned int EventLevelReload::eventId = -1;

EventLevelReload::EventLevelReload() : EventData(eventId)
{
}

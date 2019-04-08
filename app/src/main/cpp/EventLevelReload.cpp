#include "EventLevelReload.h"

const unsigned int EventLevelReload::eventId = utils::getGUID();

EventLevelReload::EventLevelReload() : EventData(eventId)
{
}

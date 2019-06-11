#pragma once

class EventData
{
private:
	unsigned int& eventId;
public:
	EventData(unsigned int& eventId) : eventId(eventId) {};
	unsigned int getEventId() const { return eventId; };
};
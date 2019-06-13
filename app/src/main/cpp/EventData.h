#pragma once

class EventData
{
private:
	int& eventId;
public:
	EventData(int& eventId) : eventId(eventId) {};
	int getEventId() const { return eventId; };
};
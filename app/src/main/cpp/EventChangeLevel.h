#pragma once

#include "EventData.h"
#include "Level.h"

struct EventChangeLevel : public EventData
{
    inline static int32_t eventId = -1;
    std::unique_ptr<Level> newLevel = nullptr;

    EventChangeLevel(std::unique_ptr<Level> newLevel) : EventData(eventId), newLevel(std::move(newLevel)) {}
};
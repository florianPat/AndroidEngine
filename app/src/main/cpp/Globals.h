#pragma once

#include "Physics.h"
#include "EventManager.h"

struct Globals
{
    static Physics* physics;
    static EventManager* eventManager;
    static Window* window;
};
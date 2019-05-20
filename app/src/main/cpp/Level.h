#pragma once

#include "Window.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "EventManager.h"
#include "Utils.h"
#include "Clock.h"

class Level
{
protected:
	Window& window;
	Graphics& gfx;
	Physics physics;
	GameObjectManager gom;
	Clock& clock;
	EventManager eventManager;

	std::unique_ptr<Level> newLevel = nullptr;
	bool endLevel = false;

	//std::function<void(EventData*)> eventLevelReloadFunction = std::bind(&Level::eventLevelReloadHandler, this, std::placeholders::_1);
	//DelegateFunction delegateLevelReload = eventManager.getDelegateFromFunction(eventLevelReloadFunction);
protected:
	void updateModelAndComposeFrame();
public:
	Level(Window& window);
	virtual ~Level() = default;
    std::unique_ptr<Level> getNewLevel();
    bool shouldEndLevel() const;
	void Go();
};
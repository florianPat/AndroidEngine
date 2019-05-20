#pragma once

#include "Level.h"
#include "TiledMap.h"

class MainLevel: public Level
{
	String tiledMapName;

	std::function<void(EventData*)> eventLevelReloadFunction = std::bind(&MainLevel::eventLevelReloadHandler, this, std::placeholders::_1);
	DelegateFunction delegateLevelReload = eventManager.getDelegateFromFunction(eventLevelReloadFunction);
private:
	void eventLevelReloadHandler(EventData* eventData);
public:
	MainLevel(Window& window, const String& tiledMapName);
};
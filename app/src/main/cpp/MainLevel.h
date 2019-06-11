#pragma once

#include "Level.h"
#include "TiledMap.h"

class MainLevel: public Level
{
	String tiledMapName;

	DelegateFunction delegateLevelReload = eventManager.getDelegateFromFunction(std::bind(&MainLevel::eventLevelReloadHandler, this, std::placeholders::_1));
private:
	void eventLevelReloadHandler(EventData* eventData);
public:
	MainLevel(const String& tiledMapName);
};
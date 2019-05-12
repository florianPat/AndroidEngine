#pragma once

#include "Level.h"

class MainLevel: public Level
{
	std::function<void(EventData*)> eventLevelReloadFunction = std::bind(&MainLevel::eventLevelReloadHandler, this, std::placeholders::_1);
	DelegateFunction delegateLevelReload = eventManager.getDelegateFromFunction(eventLevelReloadFunction);
private:
	virtual void eventLevelReloadHandler(EventData* eventData) override;
public:
	MainLevel(Window& window, const String& tiledMapName);
};
#pragma once

#include "Level.h"

class MainLevel: public Level
{
	virtual void eventLevelReloadHandler(EventData* eventData) override;
public:
	MainLevel(RenderWindow& window, String tiledMapName);
};
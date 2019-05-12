#include "MainLevel.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = std::make_unique<MainLevel>(window, levelName);
	endLevel = true;
}

MainLevel::MainLevel(Window & window, const String& tiledMapName) : Level(window, tiledMapName)
{
}

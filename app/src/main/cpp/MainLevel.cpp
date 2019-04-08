#include "MainLevel.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = std::make_unique<MainLevel>(window, levelName);
	endLevel = true;
}

MainLevel::MainLevel(RenderWindow & window, String tiledMapName) : Level(window, tiledMapName)
{
}

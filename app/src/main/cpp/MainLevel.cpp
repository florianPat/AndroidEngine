#include "MainLevel.h"
#include "EventLevelReload.h"
#include "TouchInput.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = std::make_unique<MainLevel>(window, levelName);
	endLevel = true;
}

MainLevel::MainLevel(Window & window, const String& tiledMapName) : Level(window, tiledMapName)
{
	eventManager.addListener(EventLevelReload::eventId, delegateLevelReload);
}

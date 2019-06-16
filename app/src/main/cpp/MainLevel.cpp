#include "MainLevel.h"
#include "EventLevelReload.h"
#include "TiledMapComponent.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = std::make_unique<MainLevel>(tiledMapName);
	endLevel = true;
}

MainLevel::MainLevel(const String& tiledMapName) : Level(), tiledMapName(tiledMapName)
{
}

void MainLevel::init()
{
	gom.addRenderComponent<TiledMapComponent>(0, tiledMapName);

	eventManager.addListener(EventLevelReload::eventId,
	        eventManager.getDelegateFromFunction(std::bind(&MainLevel::eventLevelReloadHandler, this, std::placeholders::_1)));
}

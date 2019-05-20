#include "MainLevel.h"
#include "EventLevelReload.h"
#include "TiledMapComponent.h"

void MainLevel::eventLevelReloadHandler(EventData * eventData)
{
	newLevel = std::make_unique<MainLevel>(window, tiledMapName);
	endLevel = true;
}

MainLevel::MainLevel(Window & window, const String& tiledMapName) : Level(window), tiledMapName(tiledMapName)
{
	TiledMap::TiledMapOptions tiledMapOptions = { window };
	Actor* actor = gom.addActor();
	actor->addComponent(std::make_unique<TiledMapComponent>(tiledMapName, &tiledMapOptions, actor));

	eventManager.addListener(EventLevelReload::eventId, delegateLevelReload);
}

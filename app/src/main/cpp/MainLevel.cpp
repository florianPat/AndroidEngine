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
	Actor* actor = gom.addActor();
	actor->addComponent(std::make_unique<TiledMapComponent>(tiledMapName));

	eventManager.addListener(EventLevelReload::eventId, delegateLevelReload);
}

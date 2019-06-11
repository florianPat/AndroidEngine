#include "Level.h"
#include "Physics.h"
#include "Globals.h"

/*void Level::eventLevelReloadHandler(EventData* eventData)
{
	newLevel = std::make_unique<Level>(window);
	endLevel = true;
}*/

Level::Level()
	:	window(*Globals::window), clock(window.getClock()), physics(), eventManager(), gom(),
		gfx(window.getGfx())
{
	Globals::physics = &physics;
	Globals::eventManager = &eventManager;
}

std::unique_ptr<Level> Level::getNewLevel()
{
    endLevel = false;
	return std::move(newLevel);
}

bool Level::shouldEndLevel() const
{
	return endLevel;
}

void Level::Go()
{
	float dt = clock.getTime().asSeconds();

	gfx.clear();

	gom.updateAndDrawActors(dt);

	physics.update(dt);

	gfx.render();
}

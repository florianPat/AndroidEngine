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
}

std::unique_ptr<Level> Level::getNewLevel()
{
    endLevel = false;
    gfx.getDefaultView().setCenter(gfx.getDefaultView().getSize().x / 2.0f, gfx.getDefaultView().getSize().y / 2.0f);
    //TODO: Set zoom and rotation of defaultView to default!
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

	gfx.render();
}

void Level::setup()
{
	Globals::eventManager = &eventManager;
	init();
}

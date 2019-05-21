#include "Level.h"

/*void Level::eventLevelReloadHandler(EventData* eventData)
{
	newLevel = std::make_unique<Level>(window);
	endLevel = true;
}*/

void Level::updateModelAndComposeFrame()
{
    float dt = clock.getTime().asSeconds();
    utils::logF("%f", dt);

	gfx.clear();

	gom.updateAndDrawActors(dt);

	physics.debugRenderBodies(gfx);

    physics.update(dt);
}

Level::Level(Window & window)
	: window(window), gfx(window.getGfx()), physics(), gom(), clock(window.getClock()),
	  eventManager()
{
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
	updateModelAndComposeFrame();
	gfx.render();
}

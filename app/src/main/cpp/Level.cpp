#include "Level.h"
#include "Physics.h"
#include "Globals.h"
#include "EventChangeLevel.h"

void Level::eventChangeLevelHandler(EventData* eventData)
{
	EventChangeLevel* eventChangeLevel = (EventChangeLevel*) eventData;

	newLevel = std::move(eventChangeLevel->newLevel);
	endLevel = true;
}

Level::Level()
	:	window(*Globals::window), clock(window.getClock()), physics(), eventManager(), gom(),
		gfx(window.getGfx())
{
}

Level::Level(uint32_t gomRenderActorSize)
		:	window(*Globals::window), clock(window.getClock()), physics(), eventManager(),
			gom(gomRenderActorSize), gfx(window.getGfx())
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
#ifdef DEBUG
	Globals::window->getNativeThreadQueue().resetStartedFlushing();
#endif
	Globals::window->getNativeThreadQueue().setNextWriteIndex(0);
	Globals::window->getAudio().clear();

	Globals::eventManager = &eventManager;
	eventManager.addClearTriggerEventsJob();
	init();
	eventManager.addListener(EventChangeLevel::eventId, GET_DELEGATE_FROM(Level, &Level::eventChangeLevelHandler));
	eventManager.addListenerEventsJobs();
	clock.restart();
}

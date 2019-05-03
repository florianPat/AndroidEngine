#include "Level.h"
#include "EventLevelReload.h"
#include "TouchInput.h"
#include "AssetLoader.h"
#include "Benchmark.h"
#include <cstdlib>
#include "Ifstream.h"

/*void Level::eventLevelReloadHandler(EventData* eventData)
{
	newLevel = std::make_unique<Level>(window, levelName);
	endLevel = true;
}*/

void Level::updateModelAndComposeFrame()
{
	float dt = clock.getTime().asSeconds();
	utils::logF("%f", dt);

	window.clear();

	map.draw(window);

	gom.updateAndDrawActors(dt);

	//NOTE: Place for level specific update and render
    font->drawText("Hello!\nThis is text rendering992.,;(%$$", {10.0f, window.getRenderHeight() - 400.0f}, window);

	physics.debugRenderBodies(window);

    physics.update(dt);
}

Level::Level(RenderWindow & window, String tiledMapName) : window(window), physics(),
gom(), clock(window.getClock()), eventManager(), map(tiledMapName, gom, eventManager, window), levelName(tiledMapName),
r()
{
	// -- test code
	c.setFillColor(Colors::Yellow);
	c.setRadius(50.0f);

	r.setSize(30.0f, 10.0f);
	r.setPosition({ window.getRenderWidth() - r.getSize().x, window.getRenderHeight() - r.getSize().y });

	//window.play(sound);
	// -- end test code

	Benchmark benchmark = Benchmark::getBenchmark();
	AssetManager* assetManager = window.getAssetManager();
	std::srand(5);

	benchmark.start("Asset loading");
	for (int i = 0; i < NUM_TEXTURES; ++i)
	{
		texture[i] = assetManager->getOrAddRes<Texture>(textureNames[std::rand() % 7]);
	}
	sound[0] = assetManager->getOrAddRes<Sound>("nice.wav");
	benchmark.stop();

	benchmark.start("One asset load");
	for (int i = 1; i < (100 - NUM_TEXTURES); ++i)
	{
		sound[i] = assetManager->getOrAddRes<Sound>("nice.wav");
	}
	benchmark.stop();

	benchmark.start("Is asset loaded");
	for (int i = 0; i < 30; ++i)
	{
		int index = std::rand() % 10;
		if (index < 8)
			assetManager->isLoaded(textureNames[index]);
		else
			assetManager->isLoaded("hahah.png");
	}
	benchmark.stop();

	benchmark.start("Reload assets");
	assetManager->reloadAllRes();
	benchmark.stop();

	benchmark.start("Clear assets");
	assetManager->clear();
	benchmark.stop();

	/*Results:
		Asset loading: has taken: 0.046008
		One asset loading: has taken: 0.000069
		Is asset loaded: has taken: 0.000077
		Reload all assets: has taken: 0.125085
		Clear assets: has taken: 0.000063
	*/

	// -- init after clear of assetManager
	Font::FontOptions options = { 32, window };
	font = window.getAssetManager()->getOrAddRes<Font>("fonts/framd.ttf", &options);
	// -- end of init

	eventManager.addListener(EventLevelReload::eventId, delegateLevelReload);
}

bool Level::Go()
{
	updateModelAndComposeFrame();
	//NOTE: Place for level specific drawing!
	window.render();

	return endLevel;
}

std::unique_ptr<Level> Level::getNewLevel()
{
	return std::move(newLevel);
}

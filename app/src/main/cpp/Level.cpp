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

	gfx.clear();

	map.draw(gfx);

	gom.updateAndDrawActors(dt);

    font->drawText("Hello!\nThis is text rendering992.,;(%$$", {10.0f, gfx.getRenderHeight() - 400.0f});
    sprite.setRotation(sprite.getRotation() + 100.0f * dt);
    gfx.draw(sprite);

    if(TouchInput::isTouched())
    {
        //This happens twice because the touchInput does not really update itself or there is a too
        // small time interval
        eventManager.TriggerEvent(std::make_unique<EventLevelReload>());
    }

	physics.debugRenderBodies(gfx);

    physics.update(dt);
}

Level::Level(Window & window, String tiledMapName)
	: window(window), gfx(window.getGfx()), physics(), gom(), clock(window.getClock()),
	  eventManager(), map(tiledMapName, gom, window), levelName(tiledMapName)
{
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

    Font::FontOptions options = { 32, &this->window };
    font = window.getAssetManager()->getOrAddRes<Font>("fonts/framd.ttf", &options);

    spriteTexture = window.getAssetManager()->getOrAddRes<Texture>("Truhe.png");
    new (&sprite) Sprite(spriteTexture);
    sprite.setScale(2.0f);
    sprite.setOrigin(sprite.getSize() / 2.0f);
    sprite.setPosition(300.0f, 400.0f);
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

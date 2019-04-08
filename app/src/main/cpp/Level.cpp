#include "Level.h"
#include "EventLevelReload.h"
#include "TouchInput.h"
#include "AssetLoader.h"
#include "Benchmark.h"
#include <cstdlib>
#include <ft2build.h>
#include FT_FREETYPE_H
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

	physics.debugRenderBodies(window);

	physics.update(dt);
}

Level::Level(RenderWindow & window, String tiledMapName) : window(window), physics(),
gom(), clock(window.getClock()), eventManager(), map(tiledMapName, gom, eventManager, window), levelName(tiledMapName),
r()
{
	// begin freetype2 test code
	FT_Library library;
	int error = FT_Init_FreeType(&library);
	if(error)
		utils::logBreak("could not init freetype library");

	Ifstream file("fonts/framd.ttf");
	const void* buffer = file.getFullData();

	FT_Face face;
	error = FT_New_Memory_Face(library, (const uchar*) buffer, file.getSize(), 0, &face);
	if(error)
		utils::logBreak("could not create face");

	error = FT_Set_Pixel_Sizes(face, 0, 16);
	if(error)
		utils::logBreak("could not set pixel size of face!");

	if(face->charmap == nullptr)
	{
		utils::log("no charmap found");
		error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		if(error)
		{
			utils::log("could not load a charmap");

			error = FT_Select_Charmap(face, FT_ENCODING_OLD_LATIN_2);
			if(error)
				utils::logBreak("could not load fallback charmap");
		}
	}

	error = FT_Load_Char(face, 'A', FT_LOAD_RENDER);
	if(error)
		utils::logBreak("could not load char from face");
	//or
	int glyphIndex = FT_Get_Char_Index(face, 'A');
	error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER);
	if(error)
		utils::logBreak("could not load char from face");

	int penX = 0, penY = 0;

	//draw(&face->glyph->bitmap, penX + face->glyph->bitmap_left, penY - face->glyph->bitmap_top);
	penX += face->glyph->advance.x >> 6;
	penY += face->height;

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	// End freetyp2 test code

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

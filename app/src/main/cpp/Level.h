#pragma once

#include "RenderWindow.h"
#include "TiledMap.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "EventManager.h"
#include "Utils.h"
#include "Clock.h"
#include "Sprite.h"
#include "CircleShape.h"
#include "RectangleShape.h"
#include "Texture.h"
#include "Sound.h"

class Level
{
protected:
	RenderWindow& window;
	Physics physics;
	GameObjectManager gom;
	Clock& clock;
	EventManager eventManager;
	TiledMap map;

	String levelName;

	std::unique_ptr<Level> newLevel = nullptr;
	bool endLevel = false;
	//Test
	CircleShape c;
	RectangleShape r;
	static constexpr int NUM_TEXTURES = 75;
	String textureNames[7] = {"Truhe.png", "ship.png", "assetsRaw/64x64/Brick1.png", "assetsRaw/64x64/Brick2.png",
		"assetsRaw/64x64/Brick3.png", "assetsRaw/64x64/Key.png", "assetsRaw/64x64/Schalter.png"};
	Texture* texture[NUM_TEXTURES];
	//Sprite sprite;
	Sound* sound[100 - NUM_TEXTURES];
	Font* font;
protected:
	std::function<void(EventData*)> eventLevelReloadFunction = std::bind(&Level::eventLevelReloadHandler, this, std::placeholders::_1);
	DelegateFunction delegateLevelReload = utils::getDelegateFromFunction(eventLevelReloadFunction);
protected:
	//TODO: Just pure virtual because you have to give it the class to recreate. Have not found how to not do it
	virtual void eventLevelReloadHandler(EventData* eventData) = 0;
protected:
	void updateModelAndComposeFrame();
public:
	Level(RenderWindow& window, String tiledMapName);
	virtual ~Level() = default;
	bool Go();
	std::unique_ptr<Level> getNewLevel();
};
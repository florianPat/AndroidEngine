#pragma once

#include "Window.h"
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
#include "Font.h"

class Level
{
protected:
	Window& window;
	Graphics& gfx;
	Physics physics;
	GameObjectManager gom;
	Clock& clock;
	EventManager eventManager;
	TiledMap map;

	String levelName;

	std::unique_ptr<Level> newLevel = nullptr;
	//TODO: Can I refactor it so that I do not need to test this flag every frame?
	bool endLevel = false;
	//Test
	static constexpr int NUM_TEXTURES = 75;
	String textureNames[7] = {"Truhe.png", "ship.png", "assetsRaw/64x64/Brick1.png", "assetsRaw/64x64/Brick2.png",
		"assetsRaw/64x64/Brick3.png", "assetsRaw/64x64/Key.png", "assetsRaw/64x64/Schalter.png"};
	Texture* texture[NUM_TEXTURES];
	Sound* sound[100 - NUM_TEXTURES];
    Font* font;
    Texture* spriteTexture;
    Sprite sprite;

	//std::function<void(EventData*)> eventLevelReloadFunction = std::bind(&Level::eventLevelReloadHandler, this, std::placeholders::_1);
	//DelegateFunction delegateLevelReload = eventManager.getDelegateFromFunction(eventLevelReloadFunction);
protected:
	//TODO: Just pure virtual because you have to give it the class to recreate. Have not found how to not do it
	virtual void eventLevelReloadHandler(EventData* eventData) = 0;
protected:
	void updateModelAndComposeFrame();
public:
	Level(Window& window, String tiledMapName);
	virtual ~Level() = default;
    std::unique_ptr<Level> getNewLevel();
    bool shouldEndLevel() const;
	void Go();
};
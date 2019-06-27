#include "MainLevel.h"
#include "TiledMap.h"
#include "Font.h"
#include "View.h"

static void registerAssetLoaders(AssetManager* assetManager)
{
	assetManager->registerAssetLoader("png", AssetLoader::initLoader<Texture, true>());
	assetManager->registerAssetLoader("wav", AssetLoader::initLoader<Sound, false>());
	assetManager->registerAssetLoader("ttf", AssetLoader::initLoader<Font, true>());
	assetManager->registerAssetLoader("tmx", AssetLoader::initLoader<TiledMap, true>());
}

void android_main(android_app* app)
{
	Window window(app, 900, 600, View::ViewportType::EXTEND);
	registerAssetLoaders(window.getAssetManager());

	std::unique_ptr<Level> currentLevel = std::make_unique<MainLevel>("testLevel.tmx");
	currentLevel->setup();

	while (window.processEvents())
	{
		currentLevel->Go();
		if (currentLevel->shouldEndLevel())
		{
			currentLevel = currentLevel->getNewLevel();
			assert(currentLevel != nullptr);
			currentLevel->setup();
		}
	}
}
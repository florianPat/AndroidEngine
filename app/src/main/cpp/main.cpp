#include "MainLevel.h"

static void registerAssetLoaders(AssetManager* assetManager)
{
	assetManager->registerAssetLoader("png", AssetLoader::initLoader<Texture>(true));
	assetManager->registerAssetLoader("wav", AssetLoader::initLoader<Sound>(false));
}

void android_main(android_app* app)
{
	RenderWindow window(app, 900, 600, RenderWindow::ViewportType::EXTEND);
	registerAssetLoaders(window.getAssetManager());

	std::unique_ptr<Level> currentLevel = std::make_unique<MainLevel>(window, "testLevel.tmx");

	while (window.processEvents())
	{
		if (currentLevel->Go())
		{
			currentLevel = currentLevel->getNewLevel();
			if (currentLevel == nullptr)
				return;
		}
	}
}
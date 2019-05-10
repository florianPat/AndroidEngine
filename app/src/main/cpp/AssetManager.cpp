#include "AssetManager.h"
#include "Utils.h"

bool AssetManager::unloadNotUsedRes(const String & filename)
{
	auto res = ressourceCache.find(filename);
	if (res != ressourceCache.end())
	{
		AssetLoader assetLoader = assetLoaderCache.at(res->first.substr(res->first.length() - 3));

		assetLoader.destruct(res->second.get());
		ressourceCache.erase(res);
		return true;
	}
	else
		return false;
}

void AssetManager::clear()
{
	for (auto it = ressourceCache.begin(); it != ressourceCache.end(); ++it)
	{
		AssetLoader assetLoader = assetLoaderCache.at(it->first.substr(it->first.length() - 3));

		assetLoader.destruct(it->second.get());
	}
	ressourceCache.clear();
}

bool AssetManager::isLoaded(const String & filename)
{
	auto i = ressourceCache.find(filename);
	return i != ressourceCache.end();
}

void AssetManager::reloadAllRes()
{
	for (auto it = ressourceCache.begin(); it != ressourceCache.end(); ++it)
	{
		AssetLoader assetLoader = assetLoaderCache.at(it->first.substr(it->first.length() - 3));

		if(assetLoader.isGpu)
		{
			if(!assetLoader.reloadFromFile(it->second.get(), it->first))
			{
				utils::logBreak("Could not reload asset!");
			}
		}
	}
}

void AssetManager::registerAssetLoader(const String & fileExt, const AssetLoader & assetLoader)
{
	assert(assetLoaderCache.find(fileExt) == assetLoaderCache.end());

	assetLoaderCache.emplace(std::make_pair(fileExt, assetLoader));
}

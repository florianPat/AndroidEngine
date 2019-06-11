#pragma once

#include <memory>
#include <unordered_map>
#include "AssetLoader.h"
#include "Utils.h"
#include "android_native_app_glue.h"

class AssetManager
{
    struct FilenameCacheValue
    {
        size_t ressourceCacheAssetId;
        //NOTE: The pointers do not get invalidated in shrink/expand because I just store a pp
        std::unique_ptr<char[]> asset;
    };

    struct RessourceCacheAssetVector
    {
        const String& filename;
        char* assetP;
    };
private:
	static constexpr long long maxSize = Gigabyte(1);
	long long currentSize = 0;
	std::unordered_map<String, FilenameCacheValue> filenameCache;
	std::unordered_map<String, int> assetLoaderCache;
	Vector<std::pair<AssetLoader, Vector<RessourceCacheAssetVector>>> ressourceCache;
public:
    AssetManager() = default;
    //TODO: Move from argOptions to just passing and forwarding them (but template variadic function
    // do not work, because there sits a function pointer in the AssetLoader)
	template <typename T>
	T* getOrAddRes(const String& filename, void* argOptions = nullptr);
	bool unloadNotUsedRes(const String& filename);
	void clear();
	bool isLoaded(const String& filename);
	void reloadAllRes();
	void registerAssetLoader(const String& fileExt, const AssetLoader& assetLoader);
};

template<typename T>
T * AssetManager::getOrAddRes(const String & filename, void* argOptions)
{
	auto res = filenameCache.find(filename);
	if (res != filenameCache.end())
	{
		T* asset = (T*) res->second.asset.get();
		return asset;
	}
	else
	{
		std::unique_ptr<char[]> asset = std::make_unique<char[]>(sizeof(T));
		T* tP = (T*) asset.get();
		new (tP) T();
		String ext = filename.substr(filename.length() - 3);
		assert(assetLoaderCache.find(ext) != assetLoaderCache.end());
		int assetLoaderIndex = assetLoaderCache.at(ext);

		auto& ressourceCachePair = ressourceCache.at(assetLoaderIndex);

		AssetLoader& assetLoader = ressourceCachePair.first;

		//(TODO: Think about how to let the user also construct a asset from a stream)
		if (!assetLoader.loadFromFile(asset.get(), filename, argOptions))
		{
			utils::logBreak("Could not load asset!");
			return nullptr;
		}

		currentSize += assetLoader.getSize(asset.get());
		//TODO: Do something if the assetCache is full!
		if (currentSize > maxSize)
		{
			InvalidCodePath;
//			do
//			{
//				auto id = timeOfInsertCache.begin();
//				auto it = ressourceCache.find(*id);
//				assert(it != ressourceCache.end());
//				AssetLoader aL = assetLoaderCache.at(it->first.substr(it->first.length() - 3));
//				currentSize -= aL.getSize(it->second.get());
//				it->second.release();
//				ressourceCache.erase(it);
//				timeOfInsertCache.erase(id);
//			} while (currentSize > maxSize);
		}

        auto result = filenameCache.emplace(std::make_pair(filename,
                FilenameCacheValue{ ressourceCachePair.second.size(), std::move(asset)} ));
        assert(result.second);

        char* assetP = result.first->second.asset.get();

		ressourceCachePair.second.push_back(RessourceCacheAssetVector{ result.first->first, assetP });

		T* returnAsset = (T*) assetP;
		return returnAsset;
	}
}
#pragma once

#include "String.h"

struct AssetLoader
{
	bool(*loadFromFile)(char* thiz, const String& filename, void* argOptions);
	bool(*reloadFromFile)(char* thiz, const String& filename);
	long long(*getSize)(char* thiz);
	void(*destruct)(char* thiz);
	bool isGpu;
	AssetLoader() = delete;
public:
	template <typename T, bool isGpuIn>
	static constexpr AssetLoader initLoader()
	{
		AssetLoader result = { 0 };
		result.loadFromFile = [](char* thiz, const String& filename, void* argOptions = nullptr)
				{return ((T*)thiz)->loadFromFile(filename); };

		if constexpr (isGpuIn)
		    result.reloadFromFile = [](char* thiz, const String& filename) {return ((T*)thiz)->reloadFromFile(filename); };

		result.getSize = [](char* thiz) {return ((T*)thiz)->getSize(); };
		result.destruct = [](char* thiz) {((T*)thiz)->~T(); };
		result.isGpu = isGpuIn;
		return result;
	}

	template <typename T, bool isGpuIn>
	static constexpr AssetLoader initLoaderWithOptions()
    {
		AssetLoader result = { 0 };
		result.loadFromFile = [](char* thiz, const String& filename, void* argOptions = nullptr)
		{return ((T*)thiz)->loadFromFile(filename, argOptions); };

		if constexpr (isGpuIn)
			result.reloadFromFile = [](char* thiz, const String& filename) {return ((T*)thiz)->reloadFromFile(filename); };

		result.getSize = [](char* thiz) {return ((T*)thiz)->getSize(); };
		result.destruct = [](char* thiz) {((T*)thiz)->~T(); };
		result.isGpu = isGpuIn;
		return result;
    }
};
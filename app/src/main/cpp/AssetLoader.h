#pragma once

#include "String.h"

struct AssetLoader
{
	bool(*loadFromFile)(char* thiz, const String& filename);
	bool(*reloadFromFile)(char* thiz, const String& filename);
	long long(*getSize)(char* thiz);
	void(*destruct)(char* thiz);
	bool isGpu;
	AssetLoader() = delete;
public:
	template <typename T>
	static AssetLoader initLoader(bool isGpuIn)
	{
		AssetLoader result = { 0 };
		result.loadFromFile = [](char* thiz, const String& filename) {return ((T*)thiz)->loadFromFile(filename); };
		result.reloadFromFile = [](char* thiz, const String& filename) {return ((T*)thiz)->reloadFromFile(filename); };
		result.getSize = [](char* thiz) {return ((T*)thiz)->getSize(); };
		result.destruct = [](char* thiz) {((T*)thiz)->~T(); };
		result.isGpu = isGpuIn;
		return result;
	}
};
#pragma once

#include "Vector.h"
#include "String.h"
#include "Texture.h"
#include <memory>
#include <unordered_map>
#include "Vector2.h"
#include "Sprite.h"
#include "AssetManager.h"

class TextureRegion
{
	friend class TextureAtlas;

	ShortString textureAtlasFileName;
	LongString filename;
	Vector2i xy;
	Vector2i size;

	Texture* atlasTexture;
	Sprite regionSprite;
private:
	TextureRegion() = default;
	void initSprite(AssetManager* assetManager);
public:
	String getAtlasFileName() { return textureAtlasFileName; }
	String getRegionName() { return filename; }
	Vector2i getXY() { return xy; }
	Vector2i getSize() { return size; }

	void setRegion(int x, int y, int widht, int height);
	Sprite getRegion();
};

class TextureAtlas
{
	struct FileHeader
	{
		ShortString name;
		Vector2i size;
		ShortString format;
		ShortString filter[2];
		ShortString repeat;
	};
public:
	TextureAtlas(const String& filepath, AssetManager* assetManger);

	const TextureRegion* findRegion(const String& name) const;
	const std::unordered_map<String, TextureRegion>& getRegions();
	void addRegion(const TextureRegion& adder);
private:
	String getLineContentBetweeen(String& lineContent, char first, char secound);
	Vector2i getLineContentRegionValues(String& lineContent, char firstRealChar);
private:
	std::unordered_map<String, TextureRegion> textureAtlas;
	static constexpr int FILE_HEADER_LINE_SIZE = 5;
	static constexpr int FILE_LINES_PER_REGION = 7;
	FileHeader fileHeader;
	AssetManager* assetManager = nullptr;
};
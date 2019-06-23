#pragma once

#include <unordered_map>
#include "Texture.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "RenderTexture.h"
#include "AssetManager.h"
#include "Ifstream.h"

class TiledMap
{
	struct Tile
	{
		int32_t id;
		int32_t width, height;
		Texture* source;
	};
	struct Layer
	{
		ShortString name;
		int32_t width, height;
		Vector<Tile> tiles;
	};
	struct ObjectGroup
	{
		ShortString name;
		Vector<Physics::Collider> objects;
	};

	Vector<Tile> tiles;
	Vector<Layer> layers;
	std::unordered_map<ShortString, ObjectGroup> objectGroups;
	int32_t mapWidth = 0, mapHeight = 0, tileWidth = 0, tileHeight = 0;

	RenderTexture texture;
	Sprite textureSprite;
	Graphics* gfx;
public:
	bool loadFromFile(const String& filename);
	bool reloadFromFile(const String& filename);
public:
	TiledMap() = default;
	const Vector<Physics::Collider>& getObjectGroup(const ShortString& objectGroupName);
	const std::unordered_map<ShortString, ObjectGroup>& getObjectGroups();
	void draw();
	uint64_t getSize() const { return sizeof(TiledMap); }
	Vector2f getMapSize() const;
private:
	uint32_t getEndOfWord(const String& word, const String& lineContent, bool* result);
	String getLineContentBetween(String& lineContent, const String& endOfFirst, char secound);

	String ParseTiles(Ifstream& file, AssetManager* assetManager, const String& filename);
	void ParseLayer(Ifstream& file, String& lineContent);
	void ParseObjectGroups(Ifstream& file, String& lineContent);
	void MakeRenderTexture();
};
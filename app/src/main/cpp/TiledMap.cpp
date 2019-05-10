#include "TiledMap.h"
#include "Ifstream.h"
#include "Utils.h"
#include "TiledMapRenderComponent.h"
#include <cstdlib>

TiledMap::TiledMap(const String & filepath, GameObjectManager& gom, EventManager& em, RenderWindow& window, Vector<ShortString>&& toGameObjects)
	: tiles(), layers(), objectGroups(), texture(), textureSprite(), assetManager(window.getAssetManager())
{
	Ifstream file;
	file.open(filepath);

	if (!file)
	{
		utils::logBreak("Cant open file!");
	}

	file.readTempLine();

	if (!file.eof())
	{
		LongString lineContent;
		file.getline(lineContent);
		assert(utils::isWordInLine("<map", lineContent));

		if (!utils::isWordInLine("orthogonal", lineContent))
		{
			utils::logBreak("Map has to be orthogonal!");
		}

		if (!utils::isWordInLine("right-down", lineContent))
		{
			utils::logBreak("Maps render-order has to be right-down!");
		}


		mapWidth = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
		mapHeight = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

		tileWidth = atoi(getLineContentBetween(lineContent, "tilewidth", '"').c_str());
		tileHeight = atoi(getLineContentBetween(lineContent, "tileheight", '"').c_str());

		lineContent = ParseTiles(file);

		ParseLayer(file, lineContent);

		ParseObjectGroups(file, lineContent);

		if (!utils::isWordInLine("</map>", lineContent))
		{
			utils::logBreak("We should be at the end of the file!");
		}

		MakeRenderTexture(toGameObjects, gom, em, window);
	}
}

const Vector<Physics::Collider>& TiledMap::getObjectGroup(const ShortString& objectGroupName)
{
	auto result = objectGroups.find(objectGroupName);
	if (result != objectGroups.end())
		return result->second.objects;
	else
	{
		InvalidCodePath;
		return Vector<Physics::Collider>();
	}
}

const std::unordered_map<ShortString, TiledMap::ObjectGroup>& TiledMap::getObjectGroups()
{
	return objectGroups;
}

void TiledMap::draw(RenderWindow& renderWindow)
{
	renderWindow.draw(textureSprite);
}

size_t TiledMap::getEndOfWord(const String & word, const String & lineContent, bool* result)
{
	size_t o = 0;
	*result = false;
	while (o < lineContent.size() && !(*result))
	{
		String searchWord(word);
		auto it = searchWord.begin();
		o = lineContent.find(it[0], o);
		++it;
		for (; o < lineContent.size(); ++it)
		{
			if (it != searchWord.end())
			{
				if (lineContent.at(++o) == it[0])
					continue;
				else
					break;
			}
			else
			{
				*result = true;
				break;
			}
		}
	}

	return ++o;
}

String TiledMap::getLineContentBetween(String & lineContent, const String & endOfFirst, char secound)
{
	bool resultValue;
	size_t widthEndPos = getEndOfWord(endOfFirst, lineContent, &resultValue);
	if (resultValue)
	{
		lineContent.erase(0, widthEndPos += 2);

		size_t kommaPos = lineContent.find(secound);

		String result(kommaPos);

		result = lineContent.substr(0, kommaPos);

		lineContent.erase(0, ++kommaPos);

		return result;
	}
	else
		return ShortString();
}

void TiledMap::ParseLayer(Ifstream & file, String& lineContent)
{
	while (utils::isWordInLine("<layer", lineContent))
	{
		String layerName = getLineContentBetween(lineContent, "name", '"');
		int layerWidth = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
		int layerHeight = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

		layers.push_back(Layer{ layerName, layerWidth, layerHeight, Vector<Tile>() });

		Layer& currentLayer = layers.back();
		currentLayer.tiles.reserve(layerWidth * layerHeight);

		file.getline(lineContent); //  <data encoding="csv">
		if (!utils::isWordInLine("csv", lineContent))
		{
			utils::logBreak("Maps encoding has to be \"csv\"");
		}

		file.getline(lineContent); //Begin of encoding

		for (int y = 0; y < layerHeight; ++y)
		{
			for (int x = 0; x < layerWidth; ++x)
			{
				size_t kommaPos = lineContent.find(',');
				int nextTileId = atoi(lineContent.substr(0, kommaPos).c_str());
				lineContent.erase(0, ++kommaPos);

				currentLayer.tiles.push_back(tiles.at(nextTileId));
			}
			file.getline(lineContent);
		}
		assert(utils::isWordInLine("</data>", lineContent));
		file.getline(lineContent); // </layer>
		assert(utils::isWordInLine("</layer>", lineContent));
		file.getline(lineContent); //Maybe new <layer>
	}
}

void TiledMap::ParseObjectGroups(Ifstream & file, String & lineContent)
{
	//ObjectGroup
	while (utils::isWordInLine("<objectgroup", lineContent))
	{
		String objectGroupName = getLineContentBetween(lineContent, "name", '"');
		file.getline(lineContent);

		Vector<Physics::Collider> objectVector;
		while (!utils::isWordInLine("</objectgroup>", lineContent))
		{
			assert(utils::isWordInLine("<object", lineContent));

			int x = atoi(getLineContentBetween(lineContent, "x", '"').c_str());
			int y = atoi(getLineContentBetween(lineContent, "y", '"').c_str());
			int width = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
			int height = atoi(getLineContentBetween(lineContent, "height", '"').c_str());

			objectVector.push_back(FloatRect((float)x, (float)y, (float)width, (float)height));

			file.getline(lineContent);
		}
		objectGroups.emplace(objectGroupName, ObjectGroup{ objectGroupName, objectVector });

		file.getline(lineContent);
	}
}

void TiledMap::MakeRenderTexture(Vector<ShortString>& toGameObjects, GameObjectManager& gom, EventManager& em, RenderWindow& window)
{
	if (texture.create(mapWidth*tileWidth, mapHeight*tileHeight))
	{
		texture.begin(window);
		window.clear();

		for (auto it = layers.begin(); it != layers.end(); ++it)
		{
			//NOTE: posY is needed here, because renderTexture 0 is bottom, but here it is top...
			for (int y = 0, posY = mapHeight - 1; y < mapHeight; ++y, --posY)
			{
				for (int x = 0; x < mapWidth; ++x)
				{
					Layer& currentLayer = *it;
					Texture* source = currentLayer.tiles.at(mapWidth * y + x).source;
					if (source == nullptr)
						continue;
					Sprite sprite(source);
					sprite.setPosition((float)x * tileWidth, (float)posY * tileHeight);

					if(toGameObjects.empty())
						window.draw(sprite);
					else
					{
						bool toGO = false;
						for (auto toGOIt = toGameObjects.begin(); toGOIt != toGameObjects.end(); ++toGOIt)
						{
							if ((*toGOIt) == it->name)
							{
								Actor* actorP = gom.addActor();
								actorP->addComponent(std::make_unique<TiledMapRenderComponent>(sprite, window, em, actorP));
								toGO = true;
								break;
							}
						}
						if(!toGO)
							window.draw(sprite);
					}
				}
			}
		}
		texture.end(window);

		textureSprite = Sprite(&texture.getTexture());
	}
	else
	{
		utils::logBreak("Could not create Render Texture");
	}
}

String TiledMap::ParseTiles(Ifstream & file)
{
	LongString lineContent;
	file.getline(lineContent);

	bool gridInFile = false;
	LongString grid;
	file.getline(grid); // <grid...
	if (utils::isWordInLine("<grid", grid))
	{
		gridInFile = true;
	}

	while (utils::isWordInLine("<tileset", lineContent))
	{
		int firstgrid = atoi(getLineContentBetween(lineContent, "firstgid", '"').c_str());

		for (int i = tiles.size(); i < firstgrid; ++i)
		{
			tiles.push_back(Tile{i, 0, 0, nullptr});
		}

		int tileCount = atoi(getLineContentBetween(lineContent, "tilecount", '"').c_str());
		for (int i = 0; i < tileCount; ++i)
		{
			if (gridInFile)
			{
				file.getline(lineContent);
			}
			else
			{
				lineContent = grid;
				//NOTE: Just set it here, because in the next it`s it will get the next line
				gridInFile = true;
			}
			assert(utils::isWordInLine("<tile", lineContent));
			int id = atoi(getLineContentBetween(lineContent, "id", '"').c_str()) + firstgrid;

			file.getline(lineContent);
			assert(utils::isWordInLine("<image", lineContent));
			int width = atoi(getLineContentBetween(lineContent, "width", '"').c_str());
			int height = atoi(getLineContentBetween(lineContent, "height", '"').c_str());
			String source = getLineContentBetween(lineContent, "source", '"');
			assert(tiles.size() == id);
			tiles.push_back(Tile{ id, width, height, assetManager->getOrAddRes<Texture>(source) });

			file.getline(lineContent); //</tile>
			assert(utils::isWordInLine("</tile>", lineContent));
		}
		file.getline(lineContent); //</tileset>
		assert(utils::isWordInLine("</tileset>", lineContent));
		file.getline(lineContent); //Maybe new <tileset>...
	}

	return std::move(lineContent);
}

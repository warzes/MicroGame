#include "stdafx.h"
#include "GenerateMap.h"
#include "privateGenerateMap.h"

GenMap* currentMap = nullptr;

void GenerateMap::GenerateDungeons(Map& map)
{
	delete currentMap;

	currentMap = new GenMap(SizeMap, SizeMap);

	Rng rng;
	ClassicDungeon generator;
	generator.generate(*currentMap, rng);

	for (int x = 0; x < SizeMap; x++)
	{
		for (int y = 0; y < SizeMap; y++)
		{
			map.tiles[x][y].color = { 1.0f, 0.8f, 0.05f, 1.0f };

			switch (currentMap->getTile(x, y))
			{
			case GenTile::Floor:
			case GenTile::Corridor:
			case GenTile::ClosedDoor:
			case GenTile::OpenDoor:
			case GenTile::UpStairs:
			case GenTile::DownStairs:
				{
					int r = rand() % 3;
					if (r == 0)      map.tiles[x][y].type = Tile::Floor1;
					else if (r == 1) map.tiles[x][y].type = Tile::Floor2;
					else if (r == 2) map.tiles[x][y].type = Tile::Floor3;
					else             map.tiles[x][y].type = Tile::Floor4;
				}
				break;

			case GenTile::Wall:
				map.tiles[x][y].type = Tile::Wall1;
				map.tiles[x][y].IsFreeMove = false;
				break;
			default:
				break;
			}
		}
	}
}

glm::vec2 GenerateMap::GetFindPosition(const Map& map)
{
	assert(currentMap);

	int recursionLimit = 10000;

	while (recursionLimit > 0)
	{
		recursionLimit--;
		int x = rand() % (SizeMap - 1);
		int y = rand() % (SizeMap - 1);

		if (currentMap->getTile(x, y) == GenTile::Floor &&
			currentMap->getTile(x, y - 1) == GenTile::Floor && currentMap->getTile(x, y + 1) == GenTile::Floor &&
			currentMap->getTile(x - 1, y) == GenTile::Floor && currentMap->getTile(x + 1, y) == GenTile::Floor)
			return glm::vec2{ x, y };
	}

	return glm::vec2();
}
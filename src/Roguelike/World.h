#pragma once

#include "Character.h"

class Object
{
public:
	void Draw(const glm::vec2& pos);

	enum ObjectType
	{
		None,
		Tree1
	};

	ObjectType type = None;
};

class Tile
{
public:
	void Draw(const glm::vec2& pos);

	bool IsFloor() const
	{
		return type == Floor1 || type == Floor2 || type == Floor3 || type == Floor4;
	}

	bool IsWall() const
	{
		return type == Wall1;
	}

	Object* object = nullptr;

	enum TileType
	{
		None,
		Grass1,
		Grass2,
		Grass3,
		Grass4,

		Floor1,
		Floor2,
		Floor3,
		Floor4,
		Wall1,
	};
	TileType type = None;
	glm::vec4 color = glm::vec4(1.0f);

	bool moveFree = true;
};

enum class MapType
{
	Dungeons
};

class Map
{
public:
	void Create();

	void Draw(const glm::vec2& playerPos);

	bool IsFreeMove(int x, int y) const
	{
		return tiles[x][y].moveFree;
	}

	Tile tiles[SizeMap][SizeMap];

	MapType type = MapType::Dungeons;
	std::wstring name;
};



class World
{
public:
	void SetMap(const std::wstring& name);
private:
	std::vector<Map> m_map;
	unsigned m_currentMapId = 0;
	Player m_player;
};
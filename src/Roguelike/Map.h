#pragma once

#include "Npc.h"

class Player;

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

	Npc* npc = nullptr;

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
	bool IsFreeMove = true;
};

enum class MapType
{
	Dungeons
};

class Map
{
public:
	void Create(Player* player);

	void Draw();

	StopMoveEvent IsFreeMove(int x, int y) const;

	Tile tiles[SizeMap][SizeMap];

	Player* player = nullptr;

	MapType type = MapType::Dungeons;
	std::wstring name;

	std::vector<Npc> npc;
};
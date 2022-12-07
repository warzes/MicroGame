#pragma once

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
};

class Map
{
public:
	void Create();

	void Draw(const glm::vec2& playerPos);

	Tile tiles[SizeMap][SizeMap];

};
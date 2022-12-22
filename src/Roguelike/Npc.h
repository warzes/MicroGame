#pragma once

class Tile;
class Map;

enum class NpcReactionType
{
	Enemy,
	Ally,
	Neutral
};

enum class StopMoveEvent
{
	Free,
	Tile,
	Player,
	Npc
};

class Npc
{
public:
	StopMoveEvent SetPosition(Map& map, int nx, int ny);
	void Draw(const glm::vec2& pos);

	NpcReactionType reactionType = NpcReactionType::Enemy;

	Tile* tileRoot = nullptr;

	int x = 0;
	int y = 0;
};
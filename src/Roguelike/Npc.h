#pragma once

enum class NpcReactionType
{
	Enemy,
	Ally,
	Neutral
};

class Npc
{
public:
	void SetPosition(int nx, int ny) { x = nx; y = ny; }
	void Draw(const glm::vec2& pos);

	NpcReactionType reactionType = NpcReactionType::Enemy;

	int x = 0;
	int y = 0;
};
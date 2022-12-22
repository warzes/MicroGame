#include "stdafx.h"
#include "Map.h"
#include "DrawHelper.h"
#include "Character.h"
//-----------------------------------------------------------------------------
void Object::Draw(const glm::vec2& pos)
{
	if (type == Tree1)
		DrawHelper::DrawTree(pos);
}
//-----------------------------------------------------------------------------
void Tile::Draw(const glm::vec2& pos)
{
	if (object)
		object->Draw(pos);
	else if (npc)
		npc->Draw(pos);
	else
	{
		if (type == None) return;

		else if (type == Grass1) DrawHelper::DrawGrass(pos, 1);
		else if (type == Grass2) DrawHelper::DrawGrass(pos, 2);
		else if (type == Grass3) DrawHelper::DrawGrass(pos, 3);
		else if (type == Grass4) DrawHelper::DrawGrass(pos, 4);

		else if (type == Floor1) DrawHelper::DrawFloor(pos, 1, color);
		else if (type == Floor2) DrawHelper::DrawFloor(pos, 2, color);
		else if (type == Floor3) DrawHelper::DrawFloor(pos, 3, color);
		else if (type == Floor4) DrawHelper::DrawFloor(pos, 4, color);

		else if (type == Wall1) DrawHelper::DrawWall(pos, 1, color);
	}
}
//-----------------------------------------------------------------------------
void Map::Create(Player* player)
{
	this->player = player;
}
//-----------------------------------------------------------------------------
void Map::Draw()
{
	int leftMapScreen   = 1;
	int rightMapScreen  = 40;
	int topMapScreen    = 1;
	int bottomMapScreen = 31;
	DrawHelper::GetScreenWorldViewport(leftMapScreen, rightMapScreen, topMapScreen, bottomMapScreen);

	const int widthMapScreen      = (rightMapScreen - leftMapScreen);
	const int heightMapScreen     = (bottomMapScreen - topMapScreen);
	const int halfWidthMapScreen  = widthMapScreen / 2;
	const int halfHeightMapScreen = heightMapScreen / 2;

	// рисуется в пределах окна мира
	for (int x = 0; x < widthMapScreen; x++)
	{
		for (int y = 0; y < heightMapScreen; y++)
		{
			const int worldX = x + player->x - halfWidthMapScreen;
			const int worldY = y + player->y - halfHeightMapScreen + 1;
			if (worldX < 0 || worldX >= SizeMap || worldY < 0 || worldY >= SizeMap) continue;

			// не рисовать тайл под игроком
			if (worldX == player->x && worldY == player->y) continue;

			tiles[worldX][worldY].Draw(glm::vec2(x + leftMapScreen, y + topMapScreen));
		}
	}
}
//-----------------------------------------------------------------------------
StopMoveEvent Map::IsFreeMove(int x, int y) const
{
	if (player->x == x && player->y == y) return StopMoveEvent::Player;
	if (tiles[x][y].npc) return StopMoveEvent::Npc;
	if (!tiles[x][y].IsFreeMove) return StopMoveEvent::Tile;

	return StopMoveEvent::Free;
}
//-----------------------------------------------------------------------------
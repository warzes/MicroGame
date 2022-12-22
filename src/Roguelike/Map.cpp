#include "stdafx.h"
#include "Map.h"
#include "DrawHelper.h"
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
void Map::Create()
{
	for (int x = 0; x < SizeMap; x++)
	{
		for (int y = 0; y < SizeMap; y++)
		{
			int r = rand() % 3;
			if (r == 0) tiles[x][y].type = Tile::Grass1;
			else if (r == 1) tiles[x][y].type = Tile::Grass2;
			else if (r == 2) tiles[x][y].type = Tile::Grass3;
			else tiles[x][y].type = Tile::Grass4;
		}
	}

	for (int i = 0; i < 100; i++)
	{
		int x = rand() % SizeMap - 1;
		int y = rand() % SizeMap - 1;

		tiles[x][y].object = new Object;
		tiles[x][y].object->type = Object::Tree1;
	}
}
//-----------------------------------------------------------------------------
void Map::Draw(const glm::vec2& playerPos)
{
	int leftMapScreen = 1;
	int rightMapScreen = 40;
	int topMapScreen = 1;
	int bottomMapScreen = 31;
	DrawHelper::GetScreenWorldViewport(leftMapScreen, rightMapScreen, topMapScreen, bottomMapScreen);

	const int widthMapScreen = (rightMapScreen - leftMapScreen);
	const int heightMapScreen = (bottomMapScreen - topMapScreen);
	const int halfWidthMapScreen = widthMapScreen / 2;
	const int halfHeightMapScreen = heightMapScreen / 2;

	// рисуется в пределах окна мира
	for (int x = 0; x < widthMapScreen; x++)
	{
		for (int y = 0; y < heightMapScreen; y++)
		{
			const int worldX = x + playerPos.x - halfWidthMapScreen;
			const int worldY = y + playerPos.y - halfHeightMapScreen + 1;
			if (worldX < 0 || worldX >= SizeMap || worldY < 0 || worldY >= SizeMap) continue;

			// не рисовать тайл под игроком
			if (worldX == playerPos.x && worldY == playerPos.y) continue;

			////  не рисовать тайл под врагами
			//// TODO: медленно, переделать
			//bool isNotVisible = false;
			//for (size_t i = 0; i < npc.size(); i++)
			//{
			//	if (worldX == npc[i].x && worldY == npc[i].y)
			//	{
			//		isNotVisible = true;
			//		break;
			//	}
			//}
			//if (isNotVisible) continue;

			tiles[worldX][worldY].Draw(glm::vec2(x + leftMapScreen, y + topMapScreen));
		}
	}

	/*for (size_t i = 0; i < npc.size(); i++)
	{
		int leftEnemy = playerPos.x - halfWidthMapScreen;
		int rightEnemy = playerPos.x + halfWidthMapScreen;
		int topEnemy = playerPos.y - halfHeightMapScreen + 1;
		int bottomEnemy = playerPos.y + halfHeightMapScreen;

		if (npc[i].x >= leftEnemy && npc[i].x <= rightEnemy &&
			npc[i].y >= topEnemy && npc[i].y <= bottomEnemy)
		{
			npc[i].Draw(glm::vec2(npc[i].x - leftEnemy + leftMapScreen, npc[i].y - topEnemy + topMapScreen));
		}
	}*/
}
#include "stdafx.h"
#include "World.h"
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
		int x = rand() % SizeMap-1;
		int y = rand() % SizeMap-1;

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
			const int worldY = y + playerPos.y - halfHeightMapScreen+1;
			if (worldX < 0 || worldX >= SizeMap || worldY < 0 || worldY >= SizeMap) continue;

			// не рисовать тайл под игроком
			if (worldX == playerPos.x && worldY == playerPos.y) continue;

			tiles[worldX][worldY].Draw(glm::vec2(x + leftMapScreen, y + topMapScreen));
		}
	}
}
//-----------------------------------------------------------------------------
void World::SetMap(const std::wstring& name)
{
	// идея такая - у каждой карты есть название. сначала ищем в массиве карт - не было ли такой карты. если не было, то добавляем новую
	// можно было хранить в мапе, но тогда бы пришлось искать при каждом обращении
}
//-----------------------------------------------------------------------------
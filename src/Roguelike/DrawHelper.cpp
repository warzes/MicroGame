#include "stdafx.h"
#include "DrawHelper.h"
#include "Sprite.h"
#include "DrawTextHelper.h"

TODO: нужна функция которвя возвращает размер строки в пикселях
в спрайт драв нужно сделать один дип (как в дебагдраве)
в рисовалке дерева возможно другие тайлы
функции рисования должны быть в единицах игры а не пикселях. Возможно создавать ортоматрицу размером в 480/размер тайла (например 480/12)

void DrawHelper::DrawMainUI()
{
	SpriteChar::Draw({ 1 * TileSize, 1 * TileSize }, { 9, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	SpriteChar::Draw({ 38 * TileSize, 1 * TileSize }, { 11, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	SpriteChar::Draw({ 1 * TileSize, 30 * TileSize }, { 8, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	SpriteChar::Draw({ 38 * TileSize, 30 * TileSize }, { 12, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));



	std::wstring nameMap = L"Тестовая карта";
	int offstepNameX = (nameMap.length() / 2) / (16.0f / TileSize);
	int skip1 = (39.0f / 2.0f - offstepNameX);
	int skip2 = (39.0f / 2.0f + offstepNameX);

	for (int x = 2; x < 38; x++)
	{
		if (x <= skip1 || x >= skip2)
			SpriteChar::Draw({ x * TileSize, 1 * TileSize }, { 13, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SpriteChar::Draw({ x * TileSize, 30 * TileSize }, { 13, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	for (int y = 2; y < 30; y++)
	{
		SpriteChar::Draw({ 1 * TileSize, y * TileSize }, { 10, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SpriteChar::Draw({ 38 * TileSize, y * TileSize }, { 10, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	DrawTextHelper::DrawCommonText(nameMap, glm::vec2((skip1+ (16.0f / TileSize))* TileSize, 8), glm::vec3(1.0f, 1.0f, 1.0f));
}

void DrawHelper::DrawTree(const glm::vec2& pos, int num)
{
	for (int i = 0; i < num; i++)
	{
		SpriteChar::Draw({ pos.x - TileSize, pos.y - TileSize - i*TileSize }, { 7, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::Draw({ pos.x,            pos.y - TileSize - i*TileSize }, { 10, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::Draw({ pos.x + TileSize, pos.y - TileSize - i*TileSize }, { 8, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	for (int i = 0; i < num; i++)
	{
		SpriteChar::Draw({ pos.x - TileSize - i*TileSize, pos.y }, { 12, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::Draw({ pos.x + TileSize + i*TileSize, pos.y }, { 13, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	for (int i = 0; i < num; i++)
	{
		SpriteChar::Draw({ pos.x - TileSize, pos.y + TileSize + i*TileSize}, { 6, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::Draw({ pos.x,            pos.y + TileSize + i*TileSize}, { 11, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::Draw({ pos.x + TileSize, pos.y + TileSize + i*TileSize}, { 9, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	SpriteChar::Draw({ pos.x, pos.y }, { 10, 3 }, glm::vec4(1.0f, 0.8f, 0.2f, 1.0f));
}
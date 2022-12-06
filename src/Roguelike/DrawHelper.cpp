#include "stdafx.h"
#include "DrawHelper.h"
#include "Sprite.h"
#include "DrawTextHelper.h"
//-----------------------------------------------------------------------------
//TODO: нужна функция которвя возвращает размер строки в пикселях
//в рисовалке дерева возможно другие тайлы
//-----------------------------------------------------------------------------
void DrawHelper::GetScreenWorldViewport(int& left, int& right, int& top, int& bottom)
{
	left = 1;
	right = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 13;
	top = 1;
	bottom = ScreenHeight / TileSize - 9;
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawMainUI()
{
	std::wstring nameMap = L"Тестовая карта";

	// границы окна карты
	int leftMapScreen = 1;
	int rightMapScreen = 1;
	int topMapScreen = 1;
	int bottomMapScreen = 1;
	GetScreenWorldViewport(leftMapScreen, rightMapScreen, topMapScreen, bottomMapScreen);

	// позиция названия карты
	const int offstepNameX = (nameMap.length() / 2.0f) / (16.0f / TileSize);
	const int skip1 = (rightMapScreen / 2.0f - offstepNameX);
	const int skip2 = (rightMapScreen / 2.0f + offstepNameX);

	// отрисовка рамки окна карты
	{
		SpriteChar::Draw({ leftMapScreen, topMapScreen }, { 9, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SpriteChar::Draw({ rightMapScreen, topMapScreen }, { 11, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SpriteChar::Draw({ leftMapScreen, bottomMapScreen }, { 8, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SpriteChar::Draw({ rightMapScreen, bottomMapScreen }, { 12, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		for (int x = leftMapScreen + 1; x < rightMapScreen; x++)
		{
			if (x <= skip1 || x >= skip2)
				SpriteChar::Draw({ x, topMapScreen }, { 13, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			SpriteChar::Draw({ x, bottomMapScreen }, { 13, 13 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}

		for (int y = topMapScreen + 1; y < bottomMapScreen; y++)
		{
			SpriteChar::Draw({ leftMapScreen, y }, { 10, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			SpriteChar::Draw({ rightMapScreen, y }, { 10, 12 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	// название карты
	DrawTextHelper::DrawCommonText(
		nameMap,
		glm::vec2((skip1 + (16.0f / TileSize)) * TileSize, 8.0f * topMapScreen),
		glm::vec3(1.0f, 1.0f, 1.0f));

	// отрисовка персонажа в центре
	SpriteChar::Draw({ 
		rightMapScreen /2,
		bottomMapScreen/2}, {1, 35}, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawTree(const glm::vec2& pos, int num)
{
	for (int i = 0; i < num; i++)
	{
		SpriteChar::DrawInMapScreen({ pos.x - 1, pos.y - 1 - i*1 }, { 7, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::DrawInMapScreen({ pos.x,     pos.y - 1 - i*1 }, { 10, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::DrawInMapScreen({ pos.x + 1, pos.y - 1 - i*1 }, { 8, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	for (int i = 0; i < num; i++)
	{
		SpriteChar::DrawInMapScreen({ pos.x - 1 - i*1, pos.y }, { 12, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::DrawInMapScreen({ pos.x + 1 + i*1, pos.y }, { 13, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	for (int i = 0; i < num; i++)
	{
		SpriteChar::DrawInMapScreen({ pos.x - 1, pos.y + 1 + i*1}, { 6, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::DrawInMapScreen({ pos.x,     pos.y + 1 + i*1}, { 11, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
		SpriteChar::DrawInMapScreen({ pos.x + 1, pos.y + 1 + i*1}, { 9, 11 }, glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
	}

	SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 10, 3 }, glm::vec4(1.0f, 0.8f, 0.2f, 1.0f));
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawGrass(const glm::vec2& pos, int num)
{
	if (num == 1) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 10, 4 }, glm::vec4(0.1f, 1.0f, 0.3f, 1.0f));
	else if (num == 2) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 11, 4 }, glm::vec4(0.1f, 1.0f, 0.3f, 1.0f));
	else if (num == 3) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 14, 3 }, glm::vec4(0.1f, 1.0f, 0.3f, 1.0f));
	else if (num == 4) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 12, 3 }, glm::vec4(0.1f, 1.0f, 0.3f, 1.0f));
}
//-----------------------------------------------------------------------------
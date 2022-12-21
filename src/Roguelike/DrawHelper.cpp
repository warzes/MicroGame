#include "stdafx.h"
#include "DrawHelper.h"
#include "Sprite.h"
#include "DrawTextHelper.h"
//-----------------------------------------------------------------------------
//TODO: нужна функция которвя возвращает размер строки в пикселях
//в рисовалке дерева возможно другие тайлы
//-----------------------------------------------------------------------------
glm::mat4 DrawHelper::GetOrtho()
{
	static float screenAspect = 0.0f;
	static glm::mat4 ortho;
	if (screenAspect != GetFrameBufferAspectRatio())
	{
		screenAspect = GetFrameBufferAspectRatio();
		const float widthHeight = ScreenHeight;
		const float widthScreen = widthHeight * screenAspect;
		ortho = glm::ortho(0.0f, widthScreen, widthHeight, 0.0f, 0.0f, 1.0f);
	}
	return ortho;

#if 0 // тряска камеры
	static float tr = 0.0f;
	static bool rev = false;
	
	if (rev == false) tr += 0.03f;
	else tr -= 0.03f;
	
	if (tr > 1.0f) rev = true;
	if (tr < -1.0f) rev = false;

	glm::mat4 trans = glm::translate(glm::vec3(tr, 0.0f, 0.0f));

	return ortho * trans;
#endif
}
//-----------------------------------------------------------------------------
void DrawHelper::GetScreenWorldViewport(int& left, int& right, int& top, int& bottom)
{
	left = 1;
	right = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 13;
	top = 1;
	bottom = ScreenHeight / TileSize - 9;
}
//-----------------------------------------------------------------------------
void DrawHelper::GetScreenMiniMapViewport(int& left, int& right, int& top, int& bottom)
{
	left = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 13+1;
	right = left + 11;
	top = 1;
	bottom = 11;
}
//-----------------------------------------------------------------------------
void DrawHelper::GetScreenInfoPlayerViewport(int& left, int& right, int& top, int& bottom)
{
	left = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 13 + 1;
	right = left + 11;
	top = 12;
	bottom = ScreenHeight / TileSize -1;
}
//-----------------------------------------------------------------------------
void DrawHelper::GetScreenLogViewport(int& left, int& right, int& top, int& bottom)
{
	left = 1;
	right = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 13;
	top = ScreenHeight / TileSize - 8;
	bottom = ScreenHeight / TileSize - 1;
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawRect(int left, int right, int top, int bottom, int topSkipToLeft, int topSkipToRight, const std::wstring& text, const glm::vec4& color)
{
	SpriteChar::Draw({ left, top }, { 9, 13 }, color);
	SpriteChar::Draw({ right, top }, { 11, 12 }, color);
	SpriteChar::Draw({ left, bottom }, { 8, 13 }, color);
	SpriteChar::Draw({ right, bottom }, { 12, 12 }, color);

	for (int x = left + 1; x < right; x++)
	{
		if ((topSkipToLeft == 0 && topSkipToRight == 0) || (x <= topSkipToLeft || x >= topSkipToRight))
			SpriteChar::Draw({ x, top }, { 13, 13 }, color);
		SpriteChar::Draw({ x, bottom }, { 13, 13 }, color);
	}

	for (int y = top + 1; y < bottom; y++)
	{
		SpriteChar::Draw({ left, y }, { 10, 12 }, color);
		SpriteChar::Draw({ right, y }, { 10, 12 }, color);
	}

	if (!text.empty())
	{
		// название карты
		DrawTextHelper::DrawCommonText(
			text,
			glm::vec2((topSkipToLeft + (16.0f / TileSize)) * TileSize, 8.0f * top),
			glm::vec3(1.0f, 1.0f, 1.0f));
	}
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawMainUI()
{
	std::wstring nameMap = L"Тестовая карта";

	// границы окон
	int leftScreen = 1;
	int rightScreen = 1;
	int topScreen = 1;
	int bottomScreen = 1;
	GetScreenWorldViewport(leftScreen, rightScreen, topScreen, bottomScreen);

	// позиция названия карты
	const int offstepNameX = (nameMap.length() / 2.0f) / (16.0f / TileSize); // TODO: сделать и использовать тут функцию возврата длины строки
	const int skip1 = (rightScreen / 2.0f - offstepNameX);
	const int skip2 = (rightScreen / 2.0f + offstepNameX);
	DrawHelper::DrawRect(leftScreen, rightScreen, topScreen, bottomScreen, skip1, skip2, nameMap);

	// отрисовка персонажа в центре
	{
		const int playerX = (rightScreen - leftScreen) / 2 + leftScreen;
		const int playerY = (bottomScreen - topScreen) / 2 + topScreen - 1;
		SpriteChar::Draw({ playerX, playerY }, { 1, 35 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
	
	GetScreenMiniMapViewport(leftScreen, rightScreen, topScreen, bottomScreen);
	DrawHelper::DrawRect(leftScreen, rightScreen, topScreen, bottomScreen);

	GetScreenInfoPlayerViewport(leftScreen, rightScreen, topScreen, bottomScreen);
	DrawHelper::DrawRect(leftScreen, rightScreen, topScreen, bottomScreen);

	GetScreenLogViewport(leftScreen, rightScreen, topScreen, bottomScreen);
	DrawHelper::DrawRect(leftScreen, rightScreen, topScreen, bottomScreen);
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawBattleUI()
{
	// границы окон
	int leftScreen = 1;
	int rightScreen = ScreenHeight * GetFrameBufferAspectRatio() / TileSize - 1;
	int topScreen = 1;
	int bottomScreen = ScreenHeight / TileSize - 1;

	std::wstring text = L"Битва";
	const int offstepNameX = (text.length() / 2.0f) / (16.0f / TileSize); // TODO: сделать и использовать тут функцию возврата длины строки
	const int skip1 = (rightScreen / 2.0f - offstepNameX-2);
	const int skip2 = (rightScreen / 2.0f + offstepNameX+2);

	DrawHelper::DrawRect(leftScreen, rightScreen, topScreen, bottomScreen, skip1, skip2, text);
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
void DrawHelper::DrawEnemy(const glm::vec2& pos)
{
	SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 1, 35 }, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawFloor(const glm::vec2& pos, int num, const glm::vec4& color)
{
	if (num == 1) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 10, 4 }, color);
	else if (num == 2) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 11, 4 }, color);
	else if (num == 3) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 14, 3 }, color);
	else if (num == 4) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 12, 3 }, color);
}
//-----------------------------------------------------------------------------
void DrawHelper::DrawWall(const glm::vec2& pos, int num, const glm::vec4& color)
{
	if (num == 1) SpriteChar::DrawInMapScreen({ pos.x, pos.y }, { 3, 3 }, color);
}
//-----------------------------------------------------------------------------
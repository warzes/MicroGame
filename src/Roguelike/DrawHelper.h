#pragma once

class DrawHelper
{
public:
	static void GetScreenWorldViewport(int& left, int& right, int& top, int& bottom);
	static void GetScreenMiniMapViewport(int& left, int& right, int& top, int& bottom);
	static void GetScreenInfoPlayerViewport(int& left, int& right, int& top, int& bottom);
	static void GetScreenLogViewport(int& left, int& right, int& top, int& bottom);

	static void DrawRect(int left, int right, int top, int bottom, int topSkipToLeft = 0, int topSkipToRight = 0, const std::wstring& text = {}, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	static void DrawMainUI();

	static void DrawTree(const glm::vec2& pos, int num  =1);

	static void DrawGrass(const glm::vec2& pos, int num);

	static void DrawFloor(const glm::vec2& pos, int num, const glm::vec4& color);
	static void DrawWall(const glm::vec2& pos, int num, const glm::vec4& color);
};
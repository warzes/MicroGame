﻿#pragma once

class DrawHelper
{
public:
	static void GetScreenWorldViewport(int& left, int& right, int& top, int& bottom);

	static void DrawMainUI();

	static void DrawTree(const glm::vec2& pos, int num  =1);

	static void DrawGrass(const glm::vec2& pos, int num);


};
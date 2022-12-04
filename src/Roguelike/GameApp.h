#pragma once

#include "Sprite.h"
#include "DrawHelper.h"

//-----------------------------------------------------------------------------
bool StartGameApp()
{

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	return true;
}

void CloseGameApp()
{
}

int x = 19;
int y = 15;

float pauseStepW = 0.0f;
float pauseStepS = 0.0f;
float pauseStepA = 0.0f;
float pauseStepD = 0.0f;

float speedStep = 7.0f;


void UpdateGameApp(float deltaTime)
{
	if (IsKeyboardKeyUp(KEY_W)) pauseStepW = 0.0f;
	if (IsKeyboardKeyUp(KEY_S)) pauseStepS = 0.0f;
	if (IsKeyboardKeyUp(KEY_A)) pauseStepA = 0.0f;
	if (IsKeyboardKeyUp(KEY_D)) pauseStepD = 0.0f;

	if (IsKeyboardKeyDown(KEY_W))
	{
		if (pauseStepW > 0.0f)
		{
			pauseStepW -= speedStep * deltaTime;
		}
		else
		{
			y--;
			pauseStepW = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_S))
	{
		if (pauseStepS > 0.0f)
		{
			pauseStepS -= speedStep * deltaTime;
		}
		else
		{
			y++;
			pauseStepS = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_A))
	{
		if (pauseStepA > 0.0f)
		{
			pauseStepA -= speedStep * deltaTime;
		}
		else
		{
			x--;
			pauseStepA = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_D))
	{
		if (pauseStepD > 0.0f)
		{
			pauseStepD -= speedStep * deltaTime;
		}
		else
		{
			x++;
			pauseStepD = 1.0f;
		}
	}
}

void FrameGameApp(float deltaTime)
{
	DrawHelper::DrawMainUI();

	DrawHelper::DrawTree(glm::vec2(72, 72), 2);

	SpriteChar::Draw({ x*12, y*12 }, { 1, 35 }, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}
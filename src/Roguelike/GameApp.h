#pragma once

#include "Sprite.h"
#include "DrawHelper.h"
#include "World.h"
#include "GenerateMap.h"

Map map;

//-----------------------------------------------------------------------------
bool StartGameApp()
{
	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	SpriteChar::Init();

	//map.Create();
	GenerateMap::GenerateDungeons(map);

	return true;
}

void CloseGameApp()
{
	SpriteChar::Close();
}

int x = 19;
int y = 15;


float pauseStepW = 0.0f;
float pauseStepS = 0.0f;
float pauseStepA = 0.0f;
float pauseStepD = 0.0f;

float speedStep = 15.0f;

bool IsTurn = false;


void UpdateGameApp(float deltaTime)
{
	IsTurn = false;

	if (IsKeyboardKeyUp(KEY_W)) pauseStepW = 0.0f;
	if (IsKeyboardKeyUp(KEY_S)) pauseStepS = 0.0f;
	if (IsKeyboardKeyUp(KEY_A)) pauseStepA = 0.0f;
	if (IsKeyboardKeyUp(KEY_D)) pauseStepD = 0.0f;

	if (IsKeyboardKeyDown(KEY_W))
	{
		if (pauseStepW > 0.0f) pauseStepW -= speedStep * deltaTime;
		else
		{
			IsTurn = true;
			y--;
			pauseStepW = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_S))
	{
		if (pauseStepS > 0.0f) pauseStepS -= speedStep * deltaTime;
		else
		{
			IsTurn = true;
			y++;
			pauseStepS = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_A))
	{
		if (pauseStepA > 0.0f) pauseStepA -= speedStep * deltaTime;
		else
		{
			IsTurn = true;
			x--;
			pauseStepA = 1.0f;
		}
	}
	else if (IsKeyboardKeyDown(KEY_D))
	{
		if (pauseStepD > 0.0f) pauseStepD -= speedStep * deltaTime;
		else
		{
			IsTurn = true;
			x++;
			pauseStepD = 1.0f;
		}
	}
}

void FrameGameApp(float deltaTime)
{
	DrawHelper::DrawMainUI();

	map.Draw(glm::vec2{x,y});

	SpriteChar::Flush();
}
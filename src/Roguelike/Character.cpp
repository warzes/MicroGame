#include "stdafx.h"
#include "Character.h"
#include "World.h"
//-----------------------------------------------------------------------------
inline void stepAndPause(float& pauseVar, int& p, bool& isTurn, float t, int mod)
{
	if (pauseVar > 0.0f) pauseVar -= t;
	else
	{
		isTurn = true;
		p += mod;
		pauseVar = 1.0f;
	}
}
//-----------------------------------------------------------------------------
void Player::Update(Map& map, float deltaTime)
{
	static float pauseStepW = 0.0f;
	static float pauseStepS = 0.0f;
	static float pauseStepA = 0.0f;
	static float pauseStepD = 0.0f;
	constexpr float speedStep = 15.0f;
	static bool IsTurn = false;

	IsTurn = false;

	int newX = x;
	int newY = y;

	if (IsKeyboardKeyUp(KEY_W)) pauseStepW = 0.0f;
	if (IsKeyboardKeyUp(KEY_S)) pauseStepS = 0.0f;
	if (IsKeyboardKeyUp(KEY_A)) pauseStepA = 0.0f;
	if (IsKeyboardKeyUp(KEY_D)) pauseStepD = 0.0f;

	if (IsKeyboardKeyDown(KEY_W))
	{
		stepAndPause(pauseStepW, newY, IsTurn, speedStep * deltaTime, -1);
	}
	if (IsKeyboardKeyDown(KEY_S))
	{
		stepAndPause(pauseStepS, newY, IsTurn, speedStep * deltaTime, 1);
	}
	if (IsKeyboardKeyDown(KEY_A))
	{
		stepAndPause(pauseStepA, newX, IsTurn, speedStep * deltaTime, -1);
	}
	if (IsKeyboardKeyDown(KEY_D))
	{
		stepAndPause(pauseStepD, newX, IsTurn, speedStep * deltaTime, 1);
	}

	if (map.IsFreeMove(newX, newY))
	{
		x = newX;
		y = newY;
	}
}
//-----------------------------------------------------------------------------
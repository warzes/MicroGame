#include "stdafx.h"
#include "Character.h"
#include "World.h"
//-----------------------------------------------------------------------------
// TODO: если заработает то убрать функцию
inline void stepAndPause(int eventKey, float& timePause, int& position, bool& isTurn, float time, int positionMod)
{
	if (IsKeyboardKeyUp(eventKey)) timePause = 0.0f;

	if (IsKeyboardKeyDown(eventKey))
	{
		if (timePause > 0.0f) timePause -= time;
		else
		{
			isTurn = true;
			position += positionMod;
			timePause = 1.0f;
		}
	}
}
//-----------------------------------------------------------------------------
inline void step(int eventKey, int& position, bool& isTurn, int positionMod)
{
	if (IsKeyboardKeyDown(eventKey))
	{
		isTurn = true;
		position += positionMod;
	}
}
//-----------------------------------------------------------------------------
bool Player::Turn(Map& map, float deltaTime)
{
	//static float pauseStepW = 0.0f;
	//static float pauseStepS = 0.0f;
	//static float pauseStepA = 0.0f;
	//static float pauseStepD = 0.0f;
	//constexpr float speedStep = 15.0f;
	static bool IsTurn = false;
	IsTurn = false;
	int newX = x;
	int newY = y;

	//stepAndPause(KEY_W, pauseStepW, newY, IsTurn, speedStep * deltaTime, -1);
	//stepAndPause(KEY_S, pauseStepS, newY, IsTurn, speedStep * deltaTime, +1);
	//stepAndPause(KEY_A, pauseStepA, newX, IsTurn, speedStep * deltaTime, -1);
	//stepAndPause(KEY_D, pauseStepD, newX, IsTurn, speedStep * deltaTime, +1);

	step(KEY_W, newY, IsTurn, -1);
	step(KEY_S, newY, IsTurn, +1);
	step(KEY_A, newX, IsTurn, -1);
	step(KEY_D, newX, IsTurn, +1);

	if (IsTurn && map.IsFreeMove(newX, newY))
	{
		x = newX;
		y = newY;
		return true;
	}

	return false;
}
//-----------------------------------------------------------------------------
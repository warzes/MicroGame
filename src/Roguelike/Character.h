#pragma once

#include "CoreGameClass.h"

class Player
{
public:
	StopMoveEvent SetPosition(Map& map, int nx, int ny);

	bool Turn(Map& map, float deltaTime);

	int x;
	int y;
};


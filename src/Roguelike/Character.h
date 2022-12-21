#pragma once

class Map;

class Player
{
public:
	void SetPosition(int nx, int ny) { x = nx; y = ny; }

	bool Turn(Map& map, float deltaTime);

	int x;
	int y;
};


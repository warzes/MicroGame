#pragma once

class Map;
class Tile;

enum class StopMoveEvent
{
	Free,
	Tile,
	Player,
	Npc
};
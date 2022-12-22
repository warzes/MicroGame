#include "stdafx.h"
#include "Npc.h"
#include "DrawHelper.h"
#include "Map.h"
//-----------------------------------------------------------------------------
bool Npc::SetPosition(Map& map, int nx, int ny)
{
	if (map.tiles[nx][ny].IsSolid()) // нельзя встать в эту клетку
		return false;

	if (map.tiles[x][y].npc) map.tiles[x][y].npc = nullptr; // убираем нпс со старого тайла

	map.tiles[nx][ny].npc = this; // записываем нпс в новый тайл

	x = nx;
	y = ny;

	return true;
}
//-----------------------------------------------------------------------------
void Npc::Draw(const glm::vec2& pos)
{
	DrawHelper::DrawEnemy(pos);
}
//-----------------------------------------------------------------------------
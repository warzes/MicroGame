#include "stdafx.h"
#include "Npc.h"
#include "DrawHelper.h"
#include "Map.h"
#include "Character.h"
//-----------------------------------------------------------------------------
StopMoveEvent Npc::SetPosition(Map& map, int nx, int ny)
{
	if (map.tiles[nx][ny].IsSolid()) // нельзя встать в эту клетку
		return StopMoveEvent::Tile;

	if (map.player->x == nx && map.player->y == ny) // там стоит игрок, а значит тоже нельзя
		return StopMoveEvent::Player;

	if (map.tiles[nx][ny].npc) // там стоит нпс, а значит тоже нельзя
		return StopMoveEvent::Npc;

	if (map.tiles[x][y].npc) map.tiles[x][y].npc = nullptr; // убираем нпс со старого тайла

	map.tiles[nx][ny].npc = this; // записываем нпс в новый тайл

	x = nx;
	y = ny;

	return StopMoveEvent::Free;
}
//-----------------------------------------------------------------------------
void Npc::Draw(const glm::vec2& pos)
{
	DrawHelper::DrawEnemy(pos);
}
//-----------------------------------------------------------------------------
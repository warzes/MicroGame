#include "stdafx.h"
#include "Npc.h"
#include "DrawHelper.h"
#include "Map.h"
#include "Character.h"
//-----------------------------------------------------------------------------
StopMoveEvent Npc::SetPosition(Map& map, int nx, int ny)
{
	const StopMoveEvent curEvent = map.IsFreeMove(nx, ny);
	if (curEvent != StopMoveEvent::Free)
		return curEvent;

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
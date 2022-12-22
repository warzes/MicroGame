#include "stdafx.h"
#include "Npc.h"
#include "DrawHelper.h"
#include "Map.h"
#include "Character.h"
//-----------------------------------------------------------------------------
StopMoveEvent Npc::SetPosition(Map& map, int nx, int ny)
{
	if (map.tiles[nx][ny].IsSolid()) // ������ ������ � ��� ������
		return StopMoveEvent::Tile;

	if (map.player->x == nx && map.player->y == ny) // ��� ����� �����, � ������ ���� ������
		return StopMoveEvent::Player;

	if (map.tiles[nx][ny].npc) // ��� ����� ���, � ������ ���� ������
		return StopMoveEvent::Npc;

	if (map.tiles[x][y].npc) map.tiles[x][y].npc = nullptr; // ������� ��� �� ������� �����

	map.tiles[nx][ny].npc = this; // ���������� ��� � ����� ����

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
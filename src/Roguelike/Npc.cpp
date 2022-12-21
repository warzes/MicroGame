#include "stdafx.h"
#include "Npc.h"
#include "DrawHelper.h"
//-----------------------------------------------------------------------------
void Npc::Draw(const glm::vec2& pos)
{
	DrawHelper::DrawEnemy(pos);
}
//-----------------------------------------------------------------------------
#pragma once

#include "Sprite.h"
#include "DrawHelper.h"
#include "World.h"
#include "GenerateMap.h"
#include "Character.h"
//-----------------------------------------------------------------------------
Map map;
Player player;
//-----------------------------------------------------------------------------
bool StartGameApp()
{
	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	SpriteChar::Init();

	GenerateMap::GenerateDungeons(map);

	auto pos = GenerateMap::GetFindPosition(map);
	player.SetPosition(pos.x, pos.y);

	return true;
}
//-----------------------------------------------------------------------------
void CloseGameApp()
{
	SpriteChar::Close();
}
//-----------------------------------------------------------------------------
void UpdateGameApp(float deltaTime)
{
	player.Update(map, deltaTime);
}
//-----------------------------------------------------------------------------
void FrameGameApp(float deltaTime)
{
	DrawHelper::DrawMainUI();

	map.Draw(glm::vec2{ player.x, player.y });

	SpriteChar::Flush();
}
//-----------------------------------------------------------------------------
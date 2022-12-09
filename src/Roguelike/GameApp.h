#pragma once

#include "Sprite.h"
#include "DrawHelper.h"
#include "World.h"
#include "GenerateMap.h"
#include "Character.h"
#include "MinimapRender.h"
//-----------------------------------------------------------------------------
World world;

MinimapRender minimapRender;
//-----------------------------------------------------------------------------
bool StartGameApp()
{
	if (!minimapRender.Create())
		return false;

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
	minimapRender.Destroy();
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
	minimapRender.Draw(map, player);
}
//-----------------------------------------------------------------------------
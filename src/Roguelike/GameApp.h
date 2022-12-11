#pragma once

#include "Sprite.h"
#include "DrawHelper.h"
#include "World.h"

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

	world.SetMap(L"test");

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
	world.UpdatePlayer(deltaTime);
}
//-----------------------------------------------------------------------------
void FrameGameApp(float deltaTime)
{
	DrawHelper::DrawMainUI();
	world.Draw();

	SpriteChar::Flush();
	minimapRender.Draw(world);
}
//-----------------------------------------------------------------------------
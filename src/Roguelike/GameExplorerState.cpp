#include "stdafx.h"
#include "GameExplorerState.h"
#include "Sprite.h"
#include "DrawHelper.h"
//-----------------------------------------------------------------------------
bool GameExplorerState::Create()
{
	if (!m_minimapRender.Create())
		return false;

	SpriteChar::Init();

	m_world.SetMap(L"test");
	return true;
}
//-----------------------------------------------------------------------------
void GameExplorerState::Destroy()
{
	SpriteChar::Close();
	m_minimapRender.Destroy();
}
//-----------------------------------------------------------------------------
void GameExplorerState::Start()
{
}
//-----------------------------------------------------------------------------
void GameExplorerState::Stop()
{
}
//-----------------------------------------------------------------------------
void GameExplorerState::Update(float deltaTime)
{
	m_world.UpdatePlayer(deltaTime);
}
//-----------------------------------------------------------------------------
void GameExplorerState::Render(float deltaTime)
{
	DrawHelper::DrawMainUI();
	m_world.Draw();
	SpriteChar::Flush();
	m_minimapRender.Draw(m_world);
}
//-----------------------------------------------------------------------------
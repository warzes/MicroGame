#include "stdafx.h"
#include "GameBattleState.h"
#include "Sprite.h"
#include "DrawHelper.h"
//-----------------------------------------------------------------------------
bool GameBattleState::Create()
{
	return true;
}
//-----------------------------------------------------------------------------
void GameBattleState::Destroy()
{
}
//-----------------------------------------------------------------------------
void GameBattleState::Start()
{
}
//-----------------------------------------------------------------------------
void GameBattleState::Stop()
{
}
//-----------------------------------------------------------------------------
void GameBattleState::Update(float deltaTime)
{
}
//-----------------------------------------------------------------------------
void GameBattleState::Render(float deltaTime)
{
	DrawHelper::DrawBattleUI();
	SpriteChar::Flush();
}
//-----------------------------------------------------------------------------
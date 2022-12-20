#include "stdafx.h"
#include "GameStateManager.h"
//-----------------------------------------------------------------------------
IGameState* currentState = nullptr;
//-----------------------------------------------------------------------------
void GameStateManager::SetState(IGameState* state)
{
	if (currentState) currentState->Stop();
	currentState = state;
	if (currentState) currentState->Start();
}
//-----------------------------------------------------------------------------
void GameStateManager::Update(float deltaTime)
{
	if (!currentState) return;
	currentState->Update(deltaTime);
}
//-----------------------------------------------------------------------------
void GameStateManager::Frame(float deltaTime)
{
	if (!currentState) return;
	currentState->Render(deltaTime);
}
//-----------------------------------------------------------------------------
#pragma once

#include "IGameState.h"

class GameExplorerState;
class GameBattleState;

extern GameExplorerState gameExplorerState;
extern GameBattleState gameBattleState;

namespace GameStateManager
{
	void SetState(IGameState* state);

	void Update(float deltaTime);
	void Frame(float deltaTime);
}
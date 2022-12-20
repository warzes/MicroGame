#pragma once

#include "IGameState.h"

namespace GameStateManager
{
	void SetState(IGameState* state);

	void Update(float deltaTime);
	void Frame(float deltaTime);
}
#pragma once

#include "IGameState.h"
#include "World.h"
#include "MinimapRender.h"

class GameExplorerState final : public IGameState
{
public:
	bool Create() final;
	void Destroy() final;

	void Start() final;
	void Stop() final;

	void Update(float deltaTime) final;
	void Render(float deltaTime) final;

private:
	World m_world;
	MinimapRender m_minimapRender;
};
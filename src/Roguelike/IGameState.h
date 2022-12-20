#pragma once

class IGameState
{
public:
	virtual ~IGameState() = default;

	virtual bool Create() = 0;
	virtual void Destroy() = 0;

	virtual void Start() = 0;
	virtual void Stop() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual void Render(float deltaTime) = 0;
};
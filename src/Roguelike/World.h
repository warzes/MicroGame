#pragma once

#include "Character.h"
#include "Map.h"

// механизм фаз хода
enum class TurnStatus
{
	BeginTurn,

	Player,
	World,

	EndTurn,

	PauseTime
};

class World
{
public:
	void SetMap(const std::wstring& name);

	void Update(float deltaTime);

	void Draw();

	Player& GetPlayer() { return m_player; }
	const Player& GetPlayer() const { return m_player; }
	Map& GetCurrentMap() { return m_map[0]; }
	const Map& GetCurrentMap() const { return m_map[0]; }
private:
	std::vector<Map> m_map;
	unsigned m_currentMapId = 0;
	Player m_player;

	TurnStatus m_turn = TurnStatus::BeginTurn;

	// переменные для паузы между ходами
	float m_pauseStep = 0.0f;
	bool m_isPause = false;
};
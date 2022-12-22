#include "stdafx.h"
#include "World.h"
#include "DrawHelper.h"
#include "GenerateMap.h"
#include "GameStateManager.h"
#include "GameBattleState.h"
//-----------------------------------------------------------------------------
void World::SetMap(const std::wstring& name)
{
	// иде€ така€ - у каждой карты есть название. сначала ищем в массиве карт - не было ли такой карты. если не было, то добавл€ем новую
	// можно было хранить в мапе, но тогда бы пришлось искать при каждом обращении

	// а пока одна карта

	m_map.push_back({});

	GenerateMap::GenerateDungeons(m_map[0]);
	m_map[0].Create(&m_player);

	int maxIt = 30;
	while (maxIt > 0)
	{
		auto pos = GenerateMap::GetFindPosition(m_map[0]);
		if (m_player.SetPosition(m_map[0], pos.x, pos.y) == StopMoveEvent::Free)
			break;
		maxIt--;
	}

	m_map[0].npc.resize(3);
	for (size_t i = 0; i < m_map[0].npc.size(); i++)
	{
		maxIt = 30;
		while (maxIt > 0)
		{
			auto pos = GenerateMap::GetFindPosition(m_map[0]);
			if (m_map[0].npc[i].SetPosition(m_map[0], pos.x, pos.y) == StopMoveEvent::Free)
				break;
			maxIt--;
		}
	}

	m_turn = TurnStatus::BeginTurn;
}
//-----------------------------------------------------------------------------
void World::Update(float deltaTime)
{
	if (m_turn == TurnStatus::BeginTurn)
	{
		// „“о-нибудь что должно быть в начале хода. ј именно хилы, счетчики эффектов и т.д.
		m_turn = TurnStatus::Player;
	}
	else if (m_turn == TurnStatus::Player)
	{
		if (m_player.Turn(m_map[0], deltaTime))
		{
			m_turn = TurnStatus::World;
		}
	}
	else if (m_turn == TurnStatus::World)
	{
		// ’од мира (монстров, нпс, событий и т.д.)


		//int r = rand() % 10;
		//if (r < 5)
		//{
		//	// после битвы начинаетс€ новый ход
		//	{
		//		m_pauseStep = 0.0f;
		//		m_isPause = false;
		//		m_turn = TurnStatus::BeginTurn;
		//	}
		//	GameStateManager::SetState(&gameBattleState);
		//}

		m_turn = TurnStatus::EndTurn;
	}
	else if (m_turn == TurnStatus::EndTurn)
	{
		// „то-то что должно быть в конце	
		m_turn = TurnStatus::PauseTime;
	}
	else if (m_turn == TurnStatus::PauseTime)
	{
		// пауза перед следующим ходом
		{
			constexpr const float speedStep = 15.0f;
			if (!m_isPause)
			{
				m_isPause = true;
				m_pauseStep = 1.0f;
			}
			else
			{
				if (m_pauseStep > 0.0f)
					m_pauseStep -= speedStep * deltaTime;
				else
				{
					m_isPause = false;
					m_turn = TurnStatus::BeginTurn;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
void World::Draw()
{
	m_map[0].Draw();
}
//-----------------------------------------------------------------------------
#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

//=============================================================================
// Hash
//=============================================================================

/// 32-bit multiply and rotate hash.
class HashMurmur2A
{
public:
	void Begin(uint32_t _seed = 0)
	{
		m_hash = _seed;
		m_tail = 0;
		m_count = 0;
		m_size = 0;
	}

	void Add(const void* _data, int32_t _len);
	//void Add(const char* _data);
	//void Add(const StringView& _data);

	template<typename Ty>
	void Add(const Ty& _data)
	{
		Add(&_data, sizeof(Ty));
	}

	uint32_t End();

//private:
	uint32_t m_hash;
	uint32_t m_tail;
	uint32_t m_count;
	uint32_t m_size;
};

//=============================================================================
// Times
//=============================================================================

std::string GetCurrentTime();

class Time
{
public:
	Time();

	void Start();
	void Update();
	float GetTime();
	float DeltaTime();
	float FPS();

private:
	std::chrono::steady_clock::time_point m_begin;
	std::chrono::steady_clock::time_point m_last;
	float m_deltaTime;
};

//=============================================================================
// Logging
//=============================================================================

struct LogCreateInfo
{
	const char* FileName = "../Log.txt";
};

bool CreateLogSystem(const LogCreateInfo& createInfo);
void DestroyLogSystem();

void LogPrint(const char* str);
void LogWarning(const char* str);
void LogError(const char* str);


inline void LogPrint(const std::string& str)
{
	LogPrint(str.c_str());
}

inline void LogWarning(const std::string& str)
{
	LogWarning(str.c_str());
}

inline void LogError(const std::string& str)
{
	LogError(str.c_str());
}
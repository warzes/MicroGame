#pragma once

#include "BaseHeader.h"

//=============================================================================
// Times
//=============================================================================

std::string GetCurrentTimeString();

class Time
{
public:
	Time();

	void Start();
	void Update();
	float GetTime();
	float GetDeltaTime();
	float GetFPS();

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

void Fatal(const std::string& str);

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
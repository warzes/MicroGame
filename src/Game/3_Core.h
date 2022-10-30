#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

std::string GetCurrentTime();

// Logging

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
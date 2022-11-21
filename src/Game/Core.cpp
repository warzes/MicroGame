#include "stdafx.h"
#include "Base.h"
#include "Core.h"
//-----------------------------------------------------------------------------
#if defined(_WIN32) && defined(_DEBUG)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
#endif
//-----------------------------------------------------------------------------
//=============================================================================
// Times
//=============================================================================
//-----------------------------------------------------------------------------
std::string GetCurrentTimeString()
{
	const std::time_t rawtime = time(nullptr);
	char rawstr[26] = { 0 };
	ctime_s(rawstr, sizeof rawstr, &rawtime);
	std::string str(rawstr);
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
	return str;
}
//-----------------------------------------------------------------------------
Time::Time()
{
	Start();
}
//-----------------------------------------------------------------------------
void Time::Start()
{
	m_begin = std::chrono::high_resolution_clock::now();
	m_last = m_begin;
}
//-----------------------------------------------------------------------------
void Time::Update()
{
	auto end = std::chrono::high_resolution_clock::now();
	m_deltaTime = ((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_last).count())) * 1e-9f;
	m_last = end;
}
//-----------------------------------------------------------------------------
float Time::GetTime()
{
	auto end = std::chrono::high_resolution_clock::now();
	return ((float)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_begin).count())) * 1e-9f;
}
//-----------------------------------------------------------------------------
float Time::GetDeltaTime()
{
	return m_deltaTime;
}
//-----------------------------------------------------------------------------
float Time::GetFPS()
{
	return (1.f / m_deltaTime);
}
//-----------------------------------------------------------------------------
//=============================================================================
// Logging
//=============================================================================
//-----------------------------------------------------------------------------
namespace
{
	constexpr auto LogSeperator = "********************************************************************************************************";
#if defined(_WIN32) || defined(__linux__)
	FILE* logFile = nullptr;
#endif
}
//-----------------------------------------------------------------------------
void logPrint(const char* simplePrefix, const char* clrPrefix, const char* str)
{
#if defined(__ANDROID__)
	__android_log_write(ANDROID_LOG_INFO, "SE_APP", str);
#elif defined(__EMSCRIPTEN__)
	emscripten_log(EM_LOG_CONSOLE, "%s", str);
#else
#	if defined(_WIN32) && defined(_DEBUG)

	if (simplePrefix)
		OutputDebugStringA((std::string(simplePrefix) + str).c_str());
	else
		OutputDebugStringA(str);
	OutputDebugStringA("\n");
#	endif

	if (clrPrefix)
		puts((std::string(clrPrefix) + str).c_str());
	else
		puts(str);

	if (logFile)
	{
		if (simplePrefix)
			fputs((std::string(simplePrefix) + str).c_str(), logFile);
		else
			fputs(str, logFile);
		fputs("\n", logFile);
	}
#endif
}
//-----------------------------------------------------------------------------
bool CreateLogSystem(const LogCreateInfo& createInfo)
{
#if defined(_WIN32)
	assert(!logFile);

	errno_t fileErr;
	fileErr = fopen_s(&logFile, createInfo.FileName, "w");
	if (fileErr != 0 || !logFile)
	{
		LogError("FileLog open failed!!!");
		logFile = nullptr;
	}
#endif
	LogPrint(LogSeperator);
	LogPrint(GetCurrentTimeString() + " Log Started.");
	LogPrint(LogSeperator);
	LogPrint("");

	return true;
}
//-----------------------------------------------------------------------------
void DestroyLogSystem()
{
	LogPrint("");
	LogPrint(LogSeperator);
	LogPrint(std::string(GetCurrentTimeString()) + " Log Ended.");
	LogPrint(LogSeperator);

#if defined(_WIN32) || defined(__linux__)
	if (logFile)
	{
		fclose(logFile);
		logFile = nullptr;
	}
#endif
}
//-----------------------------------------------------------------------------
void LogPrint(const char* str)
{
	logPrint(nullptr, nullptr, str);
}
//-----------------------------------------------------------------------------
void LogWarning(const char* str)
{
	logPrint("[ WARNING ] : ", "[ \033[33mWARNING\033[0m ] : ", str);
}
//-----------------------------------------------------------------------------
void LogError(const char* str)
{
	logPrint("[ ERROR   ] : ", "[ \033[31mERROR\033[0m   ] : ", str);
}
//-----------------------------------------------------------------------------
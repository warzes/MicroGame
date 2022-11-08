#include "stdafx.h"
#include "2_Base.h"
#include "3_Core.h"

#if defined(_WIN32) && defined(_DEBUG)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
#endif

//=============================================================================
// Hash
//=============================================================================
//-----------------------------------------------------------------------------
SE_FORCE_INLINE void mmix(uint32_t& _h, uint32_t& _k)
{
	constexpr uint32_t kMurmurMul = 0x5bd1e995;
	constexpr uint32_t kMurmurRightShift = 24;

	_k *= kMurmurMul;
	_k ^= _k >> kMurmurRightShift;
	_k *= kMurmurMul;
	_h *= kMurmurMul;
	_h ^= _k;
}
//-----------------------------------------------------------------------------
SE_FORCE_INLINE void mixTail(HashMurmur2A& _self, const uint8_t*& _data, int32_t& _len)
{
	while (_len
		&& ((_len < 4) || _self.m_count)
		)
	{
		_self.m_tail |= (*_data++) << (_self.m_count * 8);

		_self.m_count++;
		_len--;

		if (_self.m_count == 4)
		{
			mmix(_self.m_hash, _self.m_tail);
			_self.m_tail = 0;
			_self.m_count = 0;
		}
	}
}
//-----------------------------------------------------------------------------
typedef uint32_t(*ReadDataFn)(const uint8_t* _data);
template<ReadDataFn FnT>
static void addData(HashMurmur2A& _self, const uint8_t* _data, int32_t _len)
{
	while (_len >= 4)
	{
		uint32_t kk = FnT(_data);

		mmix(_self.m_hash, kk);

		_data += 4;
		_len -= 4;
	}

	mixTail(_self, _data, _len);
}
//-----------------------------------------------------------------------------
SE_FORCE_INLINE uint32_t readAligned(const uint8_t* _data)
{
	return *(uint32_t*)_data;
}
//-----------------------------------------------------------------------------
SE_FORCE_INLINE uint32_t readUnaligned(const uint8_t* _data)
{
//#if SE_CPU_ENDIAN_BIG
//	return 0
//		| _data[0] << 24
//		| _data[1] << 16
//		| _data[2] << 8
//		| _data[3]
//		;
//#else
	return 0
		| _data[0]
		| _data[1] << 8
		| _data[2] << 16
		| _data[3] << 24
		;
//#endif
}
//-----------------------------------------------------------------------------
void HashMurmur2A::Add(const void* _data, int32_t _len)
{
	HashMurmur2A& self = *this;

	const uint8_t* data = (const uint8_t*)_data;

	m_size += _len;
	mixTail(self, data, _len);

	if (!base::IsAligned(data, 4))
	{
		addData<readUnaligned>(self, data, _len);
		return;
	}

	addData<readAligned>(self, data, _len);
}
//-----------------------------------------------------------------------------
uint32_t HashMurmur2A::End()
{
	constexpr uint32_t kMurmurMul = 0x5bd1e995;

	mmix(m_hash, m_tail);
	mmix(m_hash, m_size);

	m_hash ^= m_hash >> 13;
	m_hash *= kMurmurMul;
	m_hash ^= m_hash >> 15;

	return m_hash;
}
//-----------------------------------------------------------------------------
//=============================================================================
// Times
//=============================================================================
//-----------------------------------------------------------------------------
std::string GetCurrentTime()
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
float Time::DeltaTime()
{
	return m_deltaTime;
}
//-----------------------------------------------------------------------------
float Time::FPS()
{
	return (1.f / m_deltaTime);
}
//-----------------------------------------------------------------------------
//=============================================================================
// Logging
//=============================================================================
//-----------------------------------------------------------------------------
constexpr auto LogSeperator = "********************************************************************************************************";
#if defined(_WIN32) || defined(__linux__)
FILE* logFile = nullptr;
#endif

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
	LogPrint(GetCurrentTime() + " Log Started.");
	LogPrint(LogSeperator);
	LogPrint("");

	return true;
}

void DestroyLogSystem()
{
	LogPrint("");
	LogPrint(LogSeperator);
	LogPrint(std::string(GetCurrentTime()) + " Log Ended.");
	LogPrint(LogSeperator);

#if defined(_WIN32) || defined(__linux__)
	if (logFile)
	{
		fclose(logFile);
		logFile = nullptr;
	}
#endif
}

void LogPrint(const char* str)
{
	logPrint(nullptr, nullptr, str);
}

void LogWarning(const char* str)
{
	logPrint("[ WARNING ] : ", "[ \033[33mWARNING\033[0m ] : ", str);
}

void LogError(const char* str)
{
	logPrint("[ ERROR   ] : ", "[ \033[31mERROR\033[0m   ] : ", str);
}
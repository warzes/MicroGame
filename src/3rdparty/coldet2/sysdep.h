#pragma once

///////////////////////////////////////////////////
// g++ compiler on most systems
///////////////////////////////////////////////////
#ifdef GCC

unsigned get_tick_count();

///////////////////////////////////////////////////
// Windows compilers
///////////////////////////////////////////////////
#elif defined(_WIN32)
extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount(void);
inline unsigned get_tick_count() { return GetTickCount(); }

///////////////////////////////////////////////////
// MacOS 9.0.4/MacOS X.  CodeWarrior Pro 6
// Thanks to Marco Tenuti for this addition
///////////////////////////////////////////////////
#elif defined(macintosh)
typedef unsigned long DWORD;
#include <Events.h>
#define get_tick_count() ::TickCount()

#elif defined(CUSTOM)
//typedef unsigned DWORD;
double get_tick_count();
#else
#error No system specified (WIN32 GCC macintosh)
#endif
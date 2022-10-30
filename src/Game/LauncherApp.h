#pragma once

#define USE_TEST 1

#if USE_TEST

#	define TEST_1_TRIANGLES 1

#	if TEST_1_TRIANGLES
#		include "Test1Triangles.h"
#	endif

#endif // USE_TEST
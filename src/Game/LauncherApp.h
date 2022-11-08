#pragma once

#define USE_TEST 1

#if USE_TEST

#	define TEST_1_TRIANGLES 0
#	define TEST_2_TEXTUREQUADS 0
#	define TEST_3_MODEL 1

#	if TEST_1_TRIANGLES
#		include "Test1Triangles.h"
#	endif

#	if TEST_2_TEXTUREQUADS
#		include "Test2TextureQuads.h"
#	endif

#	if TEST_3_MODEL
#		include "Test3Model.h"
#	endif

#endif // USE_TEST
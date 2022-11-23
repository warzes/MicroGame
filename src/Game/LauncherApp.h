#pragma once

#define USE_TEST 1

#if USE_TEST

#	define TEST_1_TRIANGLES 0
#	define TEST_2_TEXTUREQUADS 0
#	define TEST_3_MODEL 0
#	define TEST_4_MANYMODEL 0

#	define TEST_100_SIMPLECOLLISIONS 0

#	define TEST_200_PHYSX 0
#	define TEST_201_BULLET 1

#	define TEST_N_NEW 0

#	if TEST_1_TRIANGLES
#		include "Test001Triangles.h"
#	endif

#	if TEST_2_TEXTUREQUADS
#		include "Test002TextureQuads.h"
#	endif

#	if TEST_3_MODEL
#		include "Test003Model.h"
#	endif

#	if TEST_4_MANYMODEL
#		include "Test004ManyModels.h"
#	endif

#	if TEST_100_SIMPLECOLLISIONS
#		include "Test100SimpleCollisions.h"
#	endif

#	if TEST_200_PHYSX
#		include "Test200PhysX.h"
#	endif
#	if TEST_201_BULLET
#		include "Test201Bullet.h"
#	endif


#	if TEST_N_NEW
#		include "TestNNew.h"
#	endif

#endif // USE_TEST
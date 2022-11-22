#pragma once

#include "BaseHeader.h"

#if USE_PHYSX5

//=============================================================================
// Physics System
//=============================================================================

namespace PhysicsSystem
{
	bool Create();
	void Destroy();
}

#endif // USE_PHYSX5
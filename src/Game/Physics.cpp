#include "stdafx.h"
#include "Core.h"
#include "Physics.h"
#if USE_PHYSX5
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "PhysX_static_64.lib" )
#	pragma comment( lib, "PhysXCharacterKinematic_static_64.lib" )
#	pragma comment( lib, "PhysXCommon_static_64.lib" )
#	pragma comment( lib, "PhysXCooking_static_64.lib" )
#	pragma comment( lib, "PhysXExtensions_static_64.lib" )
#	pragma comment( lib, "PhysXFoundation_static_64.lib" )
#	pragma comment( lib, "PhysXPvdSDK_static_64.lib" )
//#	pragma comment( lib, "PhysXVehicle_static_64.lib" )
//#	pragma comment( lib, "PhysXVehicle2_static_64.lib" )
#endif
//-----------------------------------------------------------------------------
namespace
{
	PxFoundation* foundation = nullptr;
	PxDefaultErrorCallback defaultErrorCallback;
	PxDefaultAllocator defaultAllocatorCallback;

	PxPhysics* physics = nullptr;
}
//-----------------------------------------------------------------------------
bool PhysicsSystem::Create()
{
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocatorCallback, defaultErrorCallback);
	if (!foundation)
	{
		LogError("Could not create PhysX foundation");
		return false;
	}

	// Create physics device	
	constexpr bool recordMemoryAllocations = false;
	PxTolerancesScale toleranceScale{};
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, toleranceScale, recordMemoryAllocations, nullptr);
	if (!physics)
	{
		LogError("Could not create PhysX device");
		return false;
	}

	return true;
}
//-----------------------------------------------------------------------------
void PhysicsSystem::Destroy()
{
	foundation->release();
}
//-----------------------------------------------------------------------------
#endif // USE_PHYSX5
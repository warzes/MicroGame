#include "stdafx.h"
#include "Core.h"
#include "Physics.h"
#if USE_PHYSX5
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "PhysXExtensions_static_64.lib" )
#	pragma comment( lib, "PhysXPvdSDK_static_64.lib" )
#	pragma comment( lib, "PhysX_static_64.lib" )
//#	pragma comment( lib, "PhysXVehicle_static_64.lib" )
//#	pragma comment( lib, "PhysXVehicle2_static_64.lib" )
#	pragma comment( lib, "PhysXCharacterKinematic_static_64.lib" )
//#	pragma comment( lib, "PhysXCooking_static_64.lib" )
#	pragma comment( lib, "PhysXCommon_static_64.lib" )
#	pragma comment( lib, "PhysXFoundation_static_64.lib" )
#endif
//-----------------------------------------------------------------------------
namespace
{
	physx::PxFoundation* foundation = nullptr;
	physx::PxDefaultErrorCallback defaultErrorCallback;
	physx::PxDefaultAllocator defaultAllocatorCallback;

	physx::PxPhysics* physics = nullptr;
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
	physx::PxTolerancesScale toleranceScale{};
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
#if USE_BULLET
// https://github.com/50C4L/portal-cpp-opengl
// SimplexEngine
//-----------------------------------------------------------------------------
//=============================================================================
// Physics System
//=============================================================================
//-----------------------------------------------------------------------------
PhysicsObject::PhysicsObject(btCollisionShape* pShape, float mass, const btVector3& initialPosition, const btQuaternion& initialRotation) 
{
	m_pShape = pShape;

	// create the initial transform
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(initialPosition);
	transform.setRotation(initialRotation);

	// create the motion state from the initial transform
	m_pMotionState = new btDefaultMotionState(transform);

	// calculate the local inertia
	btVector3 localInertia(0, 0, 0);

	// objects of infinite mass can't move or rotate
	if (mass != 0.0f)
		pShape->calculateLocalInertia(mass, localInertia);

	// create the rigid body construction info using the mass, motion state and shape
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, m_pMotionState, pShape, localInertia);

	// create the rigid body
	m_pBody = new btRigidBody(cInfo);
}
//-----------------------------------------------------------------------------
PhysicsObject::~PhysicsObject() 
{
	delete m_pBody;
	delete m_pMotionState;
	delete m_pShape;
}
//-----------------------------------------------------------------------------
void Collider::CreateBox(const glm::vec3& size)
{
	Destroy();
	m_type = Box;
	m_shape = new btBoxShape({ size.x, size.y, size.z });
}
//-----------------------------------------------------------------------------
void Collider::CreateCapsule(float radius, float height)
{
	Destroy();
	m_type = Capsule;
	m_shape = new btCapsuleShape(radius, height);
}
//-----------------------------------------------------------------------------
void Collider::CreateSphere(float radius)
{
	Destroy();
	m_type = Sphere;
	m_shape = new btSphereShape(radius);
}
//-----------------------------------------------------------------------------
void Collider::Destroy()
{
	delete m_shape;
	m_shape = nullptr;
	m_type = None;
}
//-----------------------------------------------------------------------------
Rigidbody::Rigidbody()
{
	m_transform.setIdentity();
}
//-----------------------------------------------------------------------------
Rigidbody::~Rigidbody()
{
	delete m_motion;
	delete m_body;
}
//-----------------------------------------------------------------------------
void Rigidbody::SetFromRoot(Collider& collider, float mass)
{
	// TODO: owner root
//glm::vec3 ownerPos = p_owner.transform.GetWorldPosition();
//glm::quat ownerRot = p_owner.transform.GetWorldRotation();
	m_transform.setIdentity();
	// TODO: owner root
	//m_transform.setOrigin(btVector3(ownerPos.x, ownerPos.y, ownerPos.z));
	//m_transform.setRotation(btQuaternion(ownerRot.x, ownerRot.y, ownerRot.z, ownerRot.w));

	auto shape = collider.GetShape();
	if (shape)
	{
		btVector3 inertia(0, 0, 0);
		if (mass != 0.0)
			shape->calculateLocalInertia(mass, inertia);

		m_motion = new btDefaultMotionState(m_transform);
		btRigidBody::btRigidBodyConstructionInfo info(mass, m_motion, shape, inertia);
		m_body = new btRigidBody(info);
		m_body->setCcdMotionThreshold(static_cast<float>(1e-7));
		m_body->setCcdSweptSphereRadius(0.5f);
	}
}
//-----------------------------------------------------------------------------
void Rigidbody::SetVelocity(const glm::vec3& velocity)
{
	m_body->setLinearVelocity({ velocity.x, velocity.y, velocity.z });
}
//-----------------------------------------------------------------------------
glm::vec3 Rigidbody::GetVelocity() const
{
	auto linearVelocity = m_body->getLinearVelocity();
	return { linearVelocity.getX(), linearVelocity.getY(), linearVelocity.getZ() };
}
//-----------------------------------------------------------------------------
namespace
{
	btBroadphaseInterface* pBroadphase = nullptr;
	btCollisionConfiguration* pCollisionConfiguration = nullptr;
	btCollisionDispatcher* pDispatcher = nullptr;
	btConstraintSolver* pSolver = nullptr;
	btDynamicsWorld* pWorld = nullptr;

	std::vector<PhysicsObject*> GameObjects;
	std::vector<Rigidbody*> RigidbodyObjects;
}
//-----------------------------------------------------------------------------
void moveRigidbody(Rigidbody& target)
{
	//target.GetBody()->getMotionState()->getWorldTransform(*target.GetTransform());

	//btVector3 newPosition = target.GetTransform()->getOrigin();
	//btQuaternion newRotation = target.GetTransform()->getRotation();
	//target.owner->transform.SetLocalPosition(glm::vec3(newPosition.getX(), newPosition.getY(), newPosition.getZ()));
	//target.owner->transform.SetLocalRotation(glm::quat(newRotation.getW(), newRotation.getX(), newRotation.getY(), newRotation.getZ()));
}
//-----------------------------------------------------------------------------
bool PhysicsSystem::Create()
{
	pCollisionConfiguration = new btDefaultCollisionConfiguration();
	pDispatcher = new btCollisionDispatcher(pCollisionConfiguration);
	pBroadphase = new btDbvtBroadphase();
	pSolver = new btSequentialImpulseConstraintSolver();
	pWorld = new btDiscreteDynamicsWorld(pDispatcher, pBroadphase, pSolver, pCollisionConfiguration);
	pWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));

	return true;
}
//-----------------------------------------------------------------------------
void PhysicsSystem::FixedUpdate(float deltaTime)
{
	if (pWorld)
	{
		pWorld->stepSimulation(deltaTime);

		for (auto& rigidbody : RigidbodyObjects)
		{
			if (rigidbody)
				moveRigidbody(*rigidbody);
		}
	}
}
//-----------------------------------------------------------------------------
void PhysicsSystem::Destroy()
{
	//for (int i = 0; i < GameObjects.size(); i++)
	//{
	//	delete GameObjects[i];
	//}
	//GameObjects.clear();

	delete pWorld; pWorld = nullptr;
	delete pSolver; pSolver = nullptr;
	delete pBroadphase; pBroadphase = nullptr;
	delete pDispatcher; pDispatcher = nullptr;
	delete pCollisionConfiguration; pCollisionConfiguration = nullptr;
}
//-----------------------------------------------------------------------------
void PhysicsSystem::SetGravity(const glm::vec3& gravity)
{
	if (pWorld) pWorld->setGravity({ gravity.x, gravity.y, gravity.z });
}
//-----------------------------------------------------------------------------
void Add(Rigidbody* obj)
{
	RigidbodyObjects.push_back(obj);
}
//-----------------------------------------------------------------------------
void Remove(Rigidbody* obj)
{
	// TODO:
}
//-----------------------------------------------------------------------------
PhysicsObject* PhysicsSystem::CreatePhysicsObject(btCollisionShape* pShape, const float& mass, const btVector3& initialPosition, const btQuaternion& initialRotation)
{
	PhysicsObject* pObject = new PhysicsObject(pShape, mass, initialPosition, initialRotation);
	GameObjects.push_back(pObject);

	if (pWorld) pWorld->addRigidBody(pObject->GetRigidBody());

	return pObject;
}
//-----------------------------------------------------------------------------
#endif // USE_BULLET
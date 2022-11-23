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

#if USE_BULLET

//=============================================================================
// Physics System
//=============================================================================

class PhysicsObject
{
public:
	PhysicsObject(btCollisionShape* pShape, float mass, const btVector3& initialPosition = btVector3(0, 0, 0), const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1));
	~PhysicsObject();

	btCollisionShape* GetShape() { return m_pShape; }
	btRigidBody* GetRigidBody() { return m_pBody; }
	btMotionState* GetMotionState() { return m_pMotionState; }

	void GetTransform(btScalar* transform) 
	{
		if (m_pMotionState)
		{
			btTransform trans;
			m_pMotionState->getWorldTransform(trans);
			trans.getOpenGLMatrix(transform);
		}
	}

protected:
	btCollisionShape* m_pShape;
	btRigidBody* m_pBody;
	btDefaultMotionState* m_pMotionState;
};

class Collider
{
public:
	enum Type
	{
		None,
		Box,
		Capsule,
		Sphere
	};

	void CreateBox(const glm::vec3& size);
	void CreateCapsule(float radius, float height);
	void CreateSphere(float radius);
	void Destroy();

	btCollisionShape* GetShape() { return m_shape; }

	bool IsValid() const { return !m_shape && m_type != None; }

private:
	btCollisionShape* m_shape = nullptr;
	Type m_type = None;
};

class Rigidbody
{
public:
	Rigidbody();
	~Rigidbody();

	void SetFromRoot(Collider& collider, float mass);


	void SetVelocity(const glm::vec3& velocity);
	glm::vec3 GetVelocity() const;

	btTransform* GetTransform() { return &m_transform; }
	btMotionState* GetMotion() { return m_motion; }
	btRigidBody* GetBody() { return m_body; }

private:
	btTransform m_transform = {};
	btMotionState* m_motion = nullptr;
	btRigidBody* m_body = nullptr;
};

namespace PhysicsSystem
{
	bool Create();
	void FixedUpdate(float deltaTime);
	void Destroy();

	void SetGravity(const glm::vec3& gravity);

	void Add(Rigidbody* obj);
	void Remove(Rigidbody* obj);

	PhysicsObject* CreatePhysicsObject(btCollisionShape* pShape, 
		const float& mass,
		const btVector3& initialPosition = btVector3(0.0f, 0.0f, 0.0f),
		const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1));
}

#endif // USE_BULLET
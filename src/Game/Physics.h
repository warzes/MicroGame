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

	void Set(const glm::vec3& position, const glm::quat& rotations);
	void Set(Collider& collider, float mass);
	void Set(const glm::vec3& position, const glm::quat& rotations, Collider& collider, float mass);

	void SetVelocity(const glm::vec3& velocity);
	glm::vec3 GetVelocity() const;

	btTransform* GetTransform() { return &m_transform; }
	btMotionState* GetMotion() { return m_motion; }
	btRigidBody* GetBody() { return m_body; }

	void GetScalarTransform(btScalar* transform)
	{
		if (m_motion)
		{
			btTransform trans;
			m_motion->getWorldTransform(trans);
			trans.getOpenGLMatrix(transform);
		}
	}

	void GetPosAndRot(glm::vec3& pos, glm::quat& rot)
	{
		if (m_body)
		{
			m_body->getMotionState()->getWorldTransform(m_transform);

			btVector3 newPosition = m_transform.getOrigin();
			btQuaternion newRotation = m_transform.getRotation();
			//target.owner->transform.SetLocalPosition(glm::vec3(newPosition.getX(), newPosition.getY(), newPosition.getZ()));
			//target.owner->transform.SetLocalRotation(glm::quat(newRotation.getW(), newRotation.getX(), newRotation.getY(), newRotation.getZ()));

			pos = { newPosition.getX(), newPosition.getY(), newPosition.getZ() };
			rot = { newRotation.getW(), newRotation.getX(), newRotation.getY(), newRotation.getZ() };

			std::string ss = std::to_string(newPosition.getX()) + "|" + std::to_string(newPosition.getY()) + "|" + std::to_string(newPosition.getZ());
			puts(ss.c_str());
		}
	}

private:
	btTransform m_transform = {};
	btMotionState* m_motion = nullptr;
	btRigidBody* m_body = nullptr;
};

class CharacterController
{
public:
	void Create(Rigidbody* rigidbody, float movementSpeed, float jumpStrength, float mouseSensitivity = 1.0f);
	void Destroy();

	void Update(float deltaTime);

private:
	void handleMouse(float deltaTime);
	void handleKeyboard(float deltaTime);

	Rigidbody* m_rigidbody;

	// Internal stuffs
	float m_mouseSensitivity;
	float m_movementSpeed;
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float m_jumpStrength;
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
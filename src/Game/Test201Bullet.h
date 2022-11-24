#pragma once

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTexCoord;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(vPos, 1.0);
	vTexCoord = aTexCoord;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;

uniform sampler2D uSampler;

out vec4 fragColor;

void main()
{
	vec4 textureClr = texture(uSampler, vTexCoord);
	if (textureClr.a < 0.02) discard;
	fragColor = textureClr;
}
)";

ShaderProgram shader;
UniformLocation worldUniform;
UniformLocation viewUniform;
UniformLocation projectionUniform;
g3d::Model model;
g3d::Model capsule;
g3d::Material material;
Transform transform;

glm::vec3 CapsulePos = { 2, 3.0f, 0.0f };

Camera ncamera;

Texture2D tex;

PhysicsObject* phGroundObj;
PhysicsObject* phRedBoxObj;
PhysicsObject* phSecondBoxObj;

Rigidbody rigidBody;
Collider capsuleCollider;
CharacterController character;

#include <BulletDynamics/Character/btCharacterControllerInterface.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class DynamicCharacterController : public btCharacterControllerInterface
{
public:
	DynamicCharacterController() {
		m_rayLambda[0] = 1.0;
		m_rayLambda[1] = 1.0;
		m_halfHeight = 1.0;
		m_turnAngle = 0.0;
		m_maxLinearVelocity = 10.0;
		m_walkVelocity = 0.1; // meters/sec
		m_turnVelocity = 1.0; // radians/sec
		m_shape = NULL;
		m_rigidBody = NULL;
	}
	void setup(btScalar height = 2.0, btScalar width = 0.25, btScalar stepHeight = 0.25) 
	{
		btVector3 spherePositions[2];
		btScalar sphereRadii[2];

		sphereRadii[0] = width;
		sphereRadii[1] = width;
		spherePositions[0] = btVector3(0.0, (height / btScalar(2.0) - width), 0.0);
		spherePositions[1] = btVector3(0.0, (-height / btScalar(2.0) + width), 0.0);

		m_halfHeight = height / btScalar(2.0);

		m_shape = new btMultiSphereShape(&spherePositions[0], &sphereRadii[0], 2);

		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(btVector3(0.0, 2.0, 0.0));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo cInfo(1.0, myMotionState, m_shape);
		m_rigidBody = new btRigidBody(cInfo);
		// kinematic vs. static doesn't work
		//m_rigidBody->setCollisionFlags( m_rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		m_rigidBody->setSleepingThresholds(0.0, 0.0);
		m_rigidBody->setAngularFactor(0.0);

	}
	void destroy() 
	{
		if (m_shape)
		{
			delete m_shape;
		}

		if (m_rigidBody)
		{
			delete m_rigidBody;
			m_rigidBody = 0;
		}
	}

	virtual void reset() {}
	virtual void warp(const btVector3& origin) {}
	virtual void registerPairCacheAndDispatcher(btOverlappingPairCache* pairCache, btCollisionDispatcher* dispatcher) {}

	btCollisionObject* getCollisionObject() { return m_rigidBody; }

	void preStep(const btCollisionWorld* collisionWorld) 
	{
		btTransform xform;
		m_rigidBody->getMotionState()->getWorldTransform(xform);
		btVector3 down = -xform.getBasis()[1];
		btVector3 forward = xform.getBasis()[2];
		down.normalize();
		forward.normalize();

		m_raySource[0] = xform.getOrigin();
		m_raySource[1] = xform.getOrigin();

		m_rayTarget[0] = m_raySource[0] + down * m_halfHeight * btScalar(1.1);
		m_rayTarget[1] = m_raySource[1] + forward * m_halfHeight * btScalar(1.1);

		class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
		{
		public:
			ClosestNotMe(btRigidBody* me) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
			{
				m_me = me;
			}

			virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
			{
				if (rayResult.m_collisionObject == m_me)
					return 1.0;

				return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace
				);
			}
		protected:
			btRigidBody* m_me;
		};

		ClosestNotMe rayCallback(m_rigidBody);

		int i = 0;
		for (i = 0; i < 2; i++)
		{
			rayCallback.m_closestHitFraction = 1.0;
			collisionWorld->rayTest(m_raySource[i], m_rayTarget[i], rayCallback);
			if (rayCallback.hasHit())
			{
				m_rayLambda[i] = rayCallback.m_closestHitFraction;
			}
			else {
				m_rayLambda[i] = 1.0;
			}
		}
	}
	void playerStep(const btCollisionWorld* collisionWorld, btScalar dt,
		int forward,
		int backward,
		int left,
		int right,
		int jump) {
		btTransform xform;
		m_rigidBody->getMotionState()->getWorldTransform(xform);

		/* Handle turning */
		if (left)
			m_turnAngle -= dt * m_turnVelocity;
		if (right)
			m_turnAngle += dt * m_turnVelocity;

		xform.setRotation(btQuaternion(btVector3(0.0, 1.0, 0.0), m_turnAngle));

		btVector3 linearVelocity = m_rigidBody->getLinearVelocity();
		btScalar speed = m_rigidBody->getLinearVelocity().length();

		btVector3 forwardDir = xform.getBasis()[2];
		forwardDir.normalize();
		btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
		btScalar walkSpeed = m_walkVelocity * dt;

		if (forward)
			walkDirection += forwardDir;
		if (backward)
			walkDirection -= forwardDir;



		if (!forward && !backward && onGround())
		{
			/* Dampen when on the ground and not being moved by the player */
			linearVelocity *= btScalar(0.2);
			m_rigidBody->setLinearVelocity(linearVelocity);
		}
		else {
			if (speed < m_maxLinearVelocity)
			{
				btVector3 velocity = linearVelocity + walkDirection * walkSpeed;
				m_rigidBody->setLinearVelocity(velocity);
			}
		}

		m_rigidBody->getMotionState()->setWorldTransform(xform);
		m_rigidBody->setCenterOfMassTransform(xform);
	}

	bool canJump() const 
	{
		return onGround();
	}
	void jump()
	{
		if (!canJump())
			return;

		btTransform xform;
		m_rigidBody->getMotionState()->getWorldTransform(xform);
		btVector3 up = xform.getBasis()[1];
		up.normalize();
		btScalar magnitude = (btScalar(1.0) / m_rigidBody->getInvMass()) * btScalar(8.0);
		m_rigidBody->applyCentralImpulse(up * magnitude);
	}

	bool onGround() const
	{
		return m_rayLambda[0] < btScalar(1.0);
	}

protected:
	btScalar m_halfHeight;
	btCollisionShape* m_shape;
	btRigidBody* m_rigidBody;

	btVector3 m_raySource[2];
	btVector3 m_rayTarget[2];
	btScalar m_rayLambda[2];
	btVector3 m_rayNormal[2];

	btScalar m_turnAngle;

	btScalar m_maxLinearVelocity;
	btScalar m_walkVelocity;
	btScalar m_turnVelocity;
};

btKinematicCharacterController* m_character;
btPairCachingGhostObject* m_ghostObject;
btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
btTriangleIndexVertexArray* m_indexVertexArrays;
btVector3* m_vertices;
static int gForward = 0;
static int gBackward = 0;
static int gLeft = 0;
static int gRight = 0;
static int gJump = 0;

void InitTest()
{
	btVector3 worldMin(-1000, -1000, -1000);
	btVector3 worldMax(1000, 1000, 1000);
	btAxisSweep3* sweepBP = new btAxisSweep3(worldMin, worldMax);

	{
		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(btVector3(0, 0, 0));

		m_ghostObject = new btPairCachingGhostObject();
		m_ghostObject->setWorldTransform(startTransform);
		sweepBP->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
		btScalar characterHeight = 1.75;
		btScalar characterWidth = 1.75;
		btConvexShape* capsule = new btCapsuleShape(characterWidth, characterHeight);
		m_ghostObject->setCollisionShape(capsule);
		m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

		btScalar stepHeight = btScalar(0.35);
		m_character = new btKinematicCharacterController(m_ghostObject, capsule, stepHeight, {0.0, 1.0, 0.0});
	}

	PhysicsSystem::GetbtDynamicsWorld()->addCollisionObject(m_ghostObject, btBroadphaseProxy::StaticFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	PhysicsSystem::GetbtDynamicsWorld()->addAction(m_character);

	//PhysicsSystem::GetbtDynamicsWorld()->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_ghostObject->getBroadphaseHandle(), PhysicsSystem::GetbtDynamicsWorld()->getDispatcher());

	m_character->reset(PhysicsSystem::GetbtDynamicsWorld());
	///WTF
	m_character->warp(btVector3(0, 40, 0));




	ncamera.Teleport(0, 4, -15);
	ncamera.LookAt(glm::vec3(0, 4, 0));
	ncamera.Enable();
	ncamera.m_speed = 3;

	// create character
	{
		capsuleCollider.CreateCapsule(0.1, 1);
		rigidBody.Set(CapsulePos, { 1.0f, 0.0f, 0.0f, 0.0f }, capsuleCollider, 1.0f);
		character.Create(&rigidBody, 6.0f, 5.0f, 1.0f);
		//PhysicsSystem::Add(&rigidBody);
	}

	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		worldUniform = shader.GetUniformVariable("uWorld");
		viewUniform = shader.GetUniformVariable("uView");
		projectionUniform = shader.GetUniformVariable("uProjection");
	}

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/crate.png");
	}

	// Load geometry
	{
		model.Create("../data/models/crate.obj");
		model.SetMaterial(material);

		capsule.Create("../data/models/capsule.obj");
		capsule.SetMaterial(material);
	}	

	// create physics
	{
		// create a ground plane
		phGroundObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(1, 1000, 1000)), 0, btVector3(0.0f, -1, 0.0f));

		//// create our original red box
		phRedBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.0f, 10.0f, 0.0f));

		// create a second box
		phSecondBoxObj = PhysicsSystem::CreatePhysicsObject(new btBoxShape(btVector3(0.5, 0.5, 0.5)), 1.0, btVector3(0.25f, 20.0f, 0.0f));
	}
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	// camera move
	{
		bool mouseLock = IsMouseButtonDown(0);
		SetMouseLock(mouseLock);

		const float xpos = GetMouseX();
		const float ypos = GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		glm::vec2 mouse = tempMath::scale2(glm::vec2((lastPosX - xpos), (lastPosY - ypos)), 200.0f * deltaTime * mouseLock);
		lastPosX = xpos;
		lastPosY = ypos;
		glm::vec3 wasdec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_A) - IsKeyboardKeyDown(KEY_D), IsKeyboardKeyDown(KEY_E) - IsKeyboardKeyDown(KEY_C), IsKeyboardKeyDown(KEY_W) - IsKeyboardKeyDown(KEY_S)), ncamera.m_speed * deltaTime);

		ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
		ncamera.Fps(mouse.x, mouse.y);
	}
	
	//character.Update(deltaTime);


	if (IsKeyboardKeyPressed(KEY_Z))
	{
		phSecondBoxObj->GetRigidBody()->applyCentralImpulse(btVector3(0.0f, 5.0f, 0.0f));
	}



	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	btScalar rawTransform[16];
	btCollisionShape* pShape;
	{
		phRedBoxObj->GetTransform(rawTransform);
		pShape = phRedBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		switch (pShape->getShapeType())
		{
		case BOX_SHAPE_PROXYTYPE:
		{
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			model.Draw();
			break;
		}
		}
	}
	{
		phSecondBoxObj->GetTransform(rawTransform);
		pShape = phSecondBoxObj->GetShape();

		shader.SetUniform(worldUniform, glm::make_mat4(rawTransform));

		switch (pShape->getShapeType())
		{
		case BOX_SHAPE_PROXYTYPE:
		{
			const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
			btVector3 halfSize = box->getHalfExtentsWithMargin();
			model.Draw();
			break;
		}
		}
	}

	DebugDraw::DrawGrid(0);

	{
		glm::vec3 tcapsulePos;
		glm::quat tcapsuleRot;
		rigidBody.GetPosAndRot(tcapsulePos, tcapsuleRot);
		//DebugDraw::DrawCapsule(glm::vec3(tcapsulePos.x, tcapsulePos.y, tcapsulePos.z), glm::vec3(tcapsulePos.x, tcapsulePos.y + 1, tcapsulePos.z), 0.2f, RED);
	}

	{
		if (IsKeyboardKeyUp(KEY_UP))
		{
			gForward = 0;
		}
		if (IsKeyboardKeyUp(KEY_DOWN))
		{
			gBackward = 0;
		}
		if (IsKeyboardKeyUp(KEY_LEFT))
		{
			gLeft = 0;
		}
		if (IsKeyboardKeyUp(KEY_RIGHT))
		{
			gRight = 0;
		}
		if (IsKeyboardKeyDown(KEY_UP))
		{
			gForward = 1;
		}
		if (IsKeyboardKeyDown(KEY_DOWN))
		{
			gBackward = 1;
		}
		if (IsKeyboardKeyDown(KEY_LEFT))
		{
			gLeft = 1;
		}
		if (IsKeyboardKeyDown(KEY_RIGHT))
		{
			gRight = 1;
		}
		if (IsKeyboardKeyDown(KEY_SPACE))
		{
			if (m_character && m_character->canJump())
				gJump = 1;
		}

		///set walkDirection for our character
		btTransform xform;
		xform = m_ghostObject->getWorldTransform();

		btVector3 forwardDir = xform.getBasis()[2];
		//	printf("forwardDir=%f,%f,%f\n",forwardDir[0],forwardDir[1],forwardDir[2]);
		btVector3 upDir = xform.getBasis()[1];
		btVector3 strafeDir = xform.getBasis()[0];
		forwardDir.normalize();
		upDir.normalize();
		strafeDir.normalize();

		btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
		btScalar walkVelocity = btScalar(1.1) * 4.0; // 4 km/h -> 1.1 m/s
		btScalar walkSpeed = walkVelocity * deltaTime;

		//rotate view
		//if (gLeft)
		//{
		//	btMatrix3x3 orn = m_ghostObject->getWorldTransform().getBasis();
		//	orn *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), 0.01));
		//	m_ghostObject->getWorldTransform().setBasis(orn);
		//}

		//if (gRight)
		//{
		//	btMatrix3x3 orn = m_ghostObject->getWorldTransform().getBasis();
		//	orn *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), -0.01));
		//	m_ghostObject->getWorldTransform().setBasis(orn);
		//}

		//if (gForward)
		//	walkDirection += forwardDir;

		//if (gBackward)
		//	walkDirection -= forwardDir;


		//m_character->setWalkDirection(walkDirection * walkSpeed);


		{
			btTransform characterWorldTrans;

			//look at the vehicle
			characterWorldTrans = m_ghostObject->getWorldTransform();
			btVector3 up = characterWorldTrans.getBasis()[1];
			btVector3 backward = -characterWorldTrans.getBasis()[2];
			up.normalize();
			backward.normalize();

			auto m_cameraTargetPosition = characterWorldTrans.getOrigin();
			auto m_cameraPosition = m_cameraTargetPosition + up * 10.0 + backward * 12.0;

			//use the convex sweep test to find a safe position for the camera (not blocked by static geometry)
			btSphereShape cameraSphere(0.2f);
			btTransform cameraFrom, cameraTo;
			cameraFrom.setIdentity();
			cameraFrom.setOrigin(characterWorldTrans.getOrigin());
			cameraTo.setIdentity();
			cameraTo.setOrigin(m_cameraPosition);

			btCollisionWorld::ClosestConvexResultCallback cb(characterWorldTrans.getOrigin(), cameraTo.getOrigin());
			//cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;

			PhysicsSystem::GetbtDynamicsWorld()->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, cb);
			if (cb.hasHit())
			{

				btScalar minFraction = cb.m_closestHitFraction;//btMax(btScalar(0.3),cb.m_closestHitFraction);
				m_cameraPosition.setInterpolate3(cameraFrom.getOrigin(), cameraTo.getOrigin(), minFraction);
			}

			glm::vec3 tPos = { m_cameraPosition.getX(), m_cameraPosition.getY(), m_cameraPosition.getZ()};


			std::string ss = std::to_string(m_cameraPosition.getX()) + "|" + std::to_string(m_cameraPosition.getY()) + "|" + std::to_string(m_cameraPosition.getZ());
			puts(ss.c_str());

			DebugDraw::DrawCapsule(glm::vec3(tPos.x, tPos.y, tPos.z), glm::vec3(tPos.x, tPos.y + 1, tPos.z), 0.2f, RED);
		}
	}

	DebugDraw::Flush(ncamera);
}
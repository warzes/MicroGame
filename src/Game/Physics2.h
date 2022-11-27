#pragma once

#include "BaseHeader.h"

#if USE_MICROPHYS

// engine config

// Whether to use a fast approximation for calculating net speed of bodies which increases performance a bit. 
#define APPROXIMATE_NET_SPEED 1

// Speed, in float per ticks, that is considered low (used e.g. for auto deactivation of bodies).
#define LOW_SPEED 30


#define FRACTIONS_PER_UNIT 1.0f
#define JOINT_SIZE_MULTIPLIER 32.0f/512.0f    // joint size is scaled (size saving)


// Maximum number of iterations to try to uncollide two colliding bodies.
#define COLLISION_RESOLUTION_ITERATIONS 16

// Margin,  by which a body will be shifted back to get out of collision.
#define COLLISION_RESOLUTION_MARGIN (FRACTIONS_PER_UNIT / 64.0f)

// Number of times a collision of nonrotating bodies with environment will be attempted to resolve. This probably won't have great performance implications as complex collisions of this kind should be relatively rare.
#define NONROTATING_COLLISION_RESOLVE_ATTEMPTS 8

// Number by which the base acceleration (FRACTIONS_PER_UNIT per tick squared) caused by the connection tension will be divided. This should be power of 2.
// TODO: проверить, и возможно поделить на 512
#define TENSION_ACCELERATION_DIVIDER 32.0f

// Limit within which acceleration caused by connection tension won't be applied.
// TODO: проверить, и возможно поделить на 512
#define TENSION_ACCELERATION_THRESHOLD 5

// Connection tension threshold after which twice as much acceleration will be applied. This helps prevent diverting joints that are "impaled" by environment.
#define TENSION_GREATER_ACCELERATION_THRESHOLD  (TENSION_ACCELERATION_THRESHOLD * 3)

// Tension limit, in TPE_Units, after which a non-soft body will be reshaped. Smaller number will keep more stable shapes but will cost more performance. 
#define RESHAPE_TENSION_LIMIT 20
// How many iterations of reshaping will be performed by the step function if the body's shape needs to be reshaped. Greater number will keep shapes more stable but will cost some performance. 
#define RESHAPE_ITERATIONS 3

// After how many ticks of low speed should a body be disabled. This mustn't be greater than 255.
#define DEACTIVATE_AFTER 128

// When a body is activated by a collision, its deactivation counter will be set to this value, i.e. after a collision the body will be prone to deactivate sooner than normally. This is to handle situations with many bodies touching each other that would normally keep activating each other, never coming to rest.
#define LIGHT_DEACTIVATION  (DEACTIVATE_AFTER - DEACTIVATE_AFTER / 10.0f)

//------------------------------------------------
// TODO: удалить
#define DISTANCE vec3Dist
#define LENGTH vec3Len
inline float nonZero(float x)
{
	return x != 0 ? x : 1;
}
inline float vec3Len(const glm::vec3& v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
inline float vec3Dist(const glm::vec3& p1, const glm::vec3& p2)
{
	return vec3Len(p1 - p2);
}
inline glm::vec3 vec3Times(glm::vec3 v, float units)
{
	v.x = (v.x * units) / FRACTIONS_PER_UNIT;
	v.y = (v.y * units) / FRACTIONS_PER_UNIT;
	v.z = (v.z * units) / FRACTIONS_PER_UNIT;
	return v;
}
inline void vec3Normalize(glm::vec3& v)
{
	float l = LENGTH(v);
	if (l == 0.0f) v = { FRACTIONS_PER_UNIT, 0, 0 };
	else
	{
		v.x = (v.x * FRACTIONS_PER_UNIT) / l;
		v.y = (v.y * FRACTIONS_PER_UNIT) / l;
		v.z = (v.z * FRACTIONS_PER_UNIT) / l;
	}
}
inline float vec3Dot(const glm::vec3& v1, const glm::vec3& v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / FRACTIONS_PER_UNIT;
}
inline glm::vec3 vec3ProjectNormalized(const glm::vec3& v, const glm::vec3& baseNormalized)
{
	float p = vec3Dot(v, baseNormalized);
	glm::vec3 r;
	r.x = (p * baseNormalized.x) / FRACTIONS_PER_UNIT;
	r.y = (p * baseNormalized.y) / FRACTIONS_PER_UNIT;
	r.z = (p * baseNormalized.z) / FRACTIONS_PER_UNIT;
	return r;
}

inline glm::vec3 vec3Project(const glm::vec3& v, glm::vec3 base)
{
	vec3Normalize(base);
	return vec3ProjectNormalized(v, base);
}
inline float _abs(float x)
{
	return x >= 0 ? x : (-1 * x);
}
// TODO: возможно поделить на 512
inline float vec3LenApprox(glm::vec3 v)
{
	// 48 sided polyhedron approximation
	if (v.x < 0) v.x *= -1;
	if (v.y < 0) v.y *= -1;
	if (v.z < 0) v.z *= -1;

	if (v.x < v.y) // order the coordinates
	{
		if (v.x < v.z)
		{
			if (v.y < v.z)
			{ // v.x < v.y < v.z
				int32_t t = v.x; v.x = v.z; v.z = t;
			}
			else
			{ // v.x < v.z < v.y
				int32_t t = v.x; v.x = v.y; v.y = t;
				t = v.z; v.z = v.y; v.y = t;
			}
		}
		else
		{ // v.z < v.x < v.y
			int32_t t = v.x; v.x = v.y; v.y = t;
		}
	}
	else
	{
		if (v.y < v.z)
		{
			if (v.x < v.z)
			{ // v.y < v.x < v.z
				int32_t t = v.y; v.y = v.z; v.z = t;
				t = v.x; v.x = v.y; v.y = t;
			}
			else
			{ // v.y < v.z < v.x
				int32_t t = v.y; v.y = v.z; v.z = t;
			}
		}
	}

	float t = (893.0f * v.x + 446.0f * v.y + 223.0f * v.z) / 1024.0f;

	return  t/512.0f;
}
// TODO: удалить
//------------------------------------------------

// Returns a connection tension, i.e. a signed percentage difference against desired length (FRACTIONS_PER_UNIT means 100%).
inline float connectionTension(float length, float desiredLength)
{
	return (length * FRACTIONS_PER_UNIT) / desiredLength - FRACTIONS_PER_UNIT;
}

inline uint8_t checkOverlapAABB(const glm::vec3& v1Min, const glm::vec3& v1Max, const glm::vec3& v2Min, const glm::vec3& v2Max)
{
	float dist;
#define test(c) \
		dist = v1Min.c + v1Max.c - v2Max.c - v2Min.c; \
		if (dist < 0) dist *= -1; \
		if (dist > v1Max.c - v1Min.c + v2Max.c - v2Min.c) return 0;

	test(x)
	test(y)
	test(z)
#undef test
	return 1;
}

// Computes the formula of a 1D collision of rigid bodies.
inline void getVelocitiesAfterCollision(float* v1, float* v2, float m1, float m2, float elasticity)
{
	// In the following a lot of TPE_F cancel out, feel free to  check if confused.

	float m1Pm2 = nonZero(m1 + m2);
	float v2Mv1 = nonZero(*v2 - *v1);

	float m1v1Pm2v2 = ((m1 * *v1) + (m2 * *v2));

	*v1 = (((elasticity * m2 / FRACTIONS_PER_UNIT) * v2Mv1) + m1v1Pm2v2) / m1Pm2;
	*v2 = (((elasticity * m1 / FRACTIONS_PER_UNIT) * -1 * v2Mv1) + m1v1Pm2v2) / m1Pm2;
}

//=============================================================================
// Physics Core Object
//=============================================================================

class Joint
{
public:
	void Set(const glm::vec3& newPos, float size = FRACTIONS_PER_UNIT)
	{
		position = newPos;
		velocity = glm::vec3(0.0f);
		size /= JOINT_SIZE_MULTIPLIER;
		sizeDivided = size;
	}

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);
	float sizeDivided = FRACTIONS_PER_UNIT;
};

#define JOINT_SIZE(joint) ((joint).sizeDivided * JOINT_SIZE_MULTIPLIER)

class Connection
{
public:
	uint16_t joint1 = 0;
	uint16_t joint2 = 0;
	float length = 0.0f; // connection's preferred length
};

void MakeBox(Joint joints[8], Connection connections[16], float width, float depth, float height, float jointSize);

// Not being updated due to low energy, "sleeping", will be woken by collisions etc.
#define BODY_FLAG_DEACTIVATED 1
// When set, the body won't rotate, will only move linearly. Here the velocity of the body's first joint is the velocity of the whole body.
#define BODY_FLAG_NONROTATING 2
// Disabled, not taking part in simulation.
#define BODY_FLAG_DISABLED 4
// Soft connections, effort won't be made to keep the body's shape.
#define BODY_FLAG_SOFT 8
// Simple connections, don't zero out antagonist forces or apply connection friction, can increase performance.
#define BODY_FLAG_SIMPLE_CONN 16
// Will never deactivate due to low energy.
#define BODY_FLAG_ALWAYS_ACTIVE 32

// Physics body made of spheres (each of same weight but possibly different radia) connected by elastic springs.
class PhysicPrimitiveBody
{
public:
	void Init(
		Joint* joints, uint8_t jointCount,
		Connection* connections, uint8_t connectionCount,
		float mass = FRACTIONS_PER_UNIT);

	// Computes the center of mass of a body. This averages the position of all joints; note that if you need, you may estimate the center of the body faster, e.g.by taking a position of a single "center joint", or averaging just 2 extreme points.
	glm::vec3 GetCenterOfMass() const;

	// Computes the minimum bounding box of given body.
	void GetAABB(glm::vec3& vMin, glm::vec3& vMax);

	// Computes a bounding sphere of a body which is not minimal but faster to compute than the minimum bounding sphere.
	void GetFastBSphere(glm::vec3& center,float& radius);

	// Moves a body (its center of mass) to given position.
	void MoveTo(glm::vec3 position);

	// Moves a body by certain offset.
	void MoveBy(const glm::vec3& offset);

	Joint* joints;
	uint16_t jointCount;
	Connection* connections;
	uint16_t connectionCount;
	float jointMass;       // mass of a single joint
	float friction;        // friction of each joint
	float elasticity;      // elasticity of each joint
	uint8_t flags;
	uint8_t deactivateCount;
};

// TODO: в будущем из этого сделать ячейку, а мир - это массив ячеек
class World
{
public:
	void SetSize(const glm::vec3& center, const glm::vec3& size);
	void SetGravity(const glm::vec3& gravity);
	void AddBody(PhysicPrimitiveBody* body);

	// Performs one step (tick, frame, ...) of the physics world simulation including updating positionsand velocities of bodies, collision detectionand resolution, possible reshaping or deactivation of inactive bodies etc.The time length of the step is relative to all other units but it's ideal if it is 1/60th of a second.
	void Tick();

	PhysicPrimitiveBody* GetBody(size_t id) { return m_bodies[id]; }

private:
	glm::vec3 environmentDistance(const glm::vec3& point, float maxDistance)
	{
		return aaboxInside(point, maxDistance);
	}

	glm::vec3 aaboxInside(glm::vec3 point, float maxDistance);
	bool bodyEnvironmentResolveCollision(PhysicPrimitiveBody* body);
	uint8_t jointEnvironmentResolveCollision(Joint* joint, float elasticity, float friction);
	void bodyNonrotatingJointCollided(PhysicPrimitiveBody* b, int16_t jointIndex, glm::vec3 origPos, uint8_t success);

	// Tests whether a body is currently colliding with the environment.
	uint8_t bodyEnvironmentCollide(const PhysicPrimitiveBody* body);

	// Attempts to shift the joints of a soft body so that the tension of all springs becomes zero while keeping the joints near their current position. This function performs one iteration of the equalizing algorithm and doesn't guarantee a perfect solution, it may help to run multiple iterations (call this function multiple times).
	void bodyReshape(PhysicPrimitiveBody* body);

	// performs some "magic" on body connections, mainly cancelling out of velocities going against each other and also applying connection friction in soft bodies. The strong parameter indicates if the body is soft or not.
	void bodyCancelOutVelocities(PhysicPrimitiveBody* body, uint8_t strong);

	// checks and potentiall resolves collision of two bodies so as to keep them outside given environment. Returns 1 if collision happened or 0 otherwise.
	uint8_t bodiesResolveCollision(PhysicPrimitiveBody* b1, PhysicPrimitiveBody* b2);

	// resolves a potential collision of two joints in a way that keeps the joints outside provided environment (if the function pointer is not 0). Returns 1 if joints collided or 0 otherwise.
	uint8_t jointsResolveCollision(Joint* j1, Joint* j2, float mass1, float mass2, float elasticity, float friction);

	void bodyActivate(PhysicPrimitiveBody* body);
	// Zeros velocities of all soft body joints.
	void bodyStop(PhysicPrimitiveBody* body);

	float bodyGetAverageSpeed(const PhysicPrimitiveBody* body);
	float bodyGetNetSpeed(const PhysicPrimitiveBody* body);

	std::vector<PhysicPrimitiveBody*> m_bodies;

	glm::vec3 m_centerWorld;
	glm::vec3 m_sizeWorld;

	glm::vec3 m_gravity = glm::vec3(0.0f);
};


//=============================================================================
// Physics System
//=============================================================================

#endif // USE_MICROPHYS
#include "stdafx.h"
#include "Core.h"
#include "Physics2.h"

#if USE_MICROPHYS
// private:
namespace
{
	uint32_t body1Index = 0;
	uint32_t body2Index = 0;
	uint32_t joint1Index = 0;
	uint32_t joint2Index = 0;
}
//-----------------------------------------------------------------------------
#define C(n,a,b) connections[n].joint1 = a; connections[n].joint2 = b;
//-----------------------------------------------------------------------------
void MakeBox(Joint joints[8], Connection connections[16], float width, float depth, float height, float jointSize)
{
	width /= 2.0f;
	depth /= 2.0f;
	height /= 2.0f;

	for (uint8_t i = 0; i < 8; ++i)
		joints[i].Set(
			glm::vec3(
				(i % 2) ? width : (-1 * width),
				((i >> 2) % 2) ? height : (-1 * height),
				((i >> 1) % 2) ? depth : (-1 * depth)),
			jointSize);

	C(0, 0, 1) C(1, 1, 3) C(2, 3, 2) C(3, 2, 0)  // top
	C(4, 4, 5) C(5, 5, 7) C(6, 7, 6) C(7, 6, 4)  // bottom
	C(8, 0, 4) C(9, 1, 5) C(10, 3, 7) C(11, 2, 6)  // middle
	C(12, 0, 7) C(13, 1, 6) C(14, 2, 5) C(15, 3, 4)  // diagonal
}
//-----------------------------------------------------------------------------
void PhysicPrimitiveBody::Init(Joint* pJoints, uint8_t JointCount, Connection* pConnections, uint8_t ConnectionCount, float mass)
{
	joints = pJoints;
	jointCount = JointCount;
	connections = pConnections;
	connectionCount = ConnectionCount;
	deactivateCount = 0;
	friction = FRACTIONS_PER_UNIT / 2.0f;
	elasticity = FRACTIONS_PER_UNIT / 2.0f;
	flags = 0;
	jointMass = mass / jointCount;
	if (jointMass == 0) jointMass = 1;

	for (uint32_t i = 0; i < connectionCount; ++i)
	{
		float d = DISTANCE(
			joints[connections[i].joint1].position,
			joints[connections[i].joint2].position);
		connections[i].length = d != 0.0f ? d : 1.0f; // prevent later division by zero
	}
}
//-----------------------------------------------------------------------------
glm::vec3 PhysicPrimitiveBody::GetCenterOfMass() const
{
	// note that joint sizes don't play a role as all weight the same

	glm::vec3 result = glm::vec3(0.0f);
	for (int i = 0; i < jointCount; i++)
		result += joints[i].position;
	result /= jointCount;

	return result;
}
//-----------------------------------------------------------------------------
void PhysicPrimitiveBody::GetAABB(glm::vec3& vMin, glm::vec3& vMax)
{
	vMin = joints[0].position;
	vMax = vMin;

	float js = JOINT_SIZE(joints[0]);

	vMin.x -= js;
	vMin.y -= js;
	vMin.z -= js;

	vMax.x += js;
	vMax.y += js;
	vMax.z += js;

	for (uint16_t i = 1; i < jointCount; ++i)
	{
		float v;
		js = JOINT_SIZE(joints[i]);
#define test(c) \
			v = joints[i].position.c - js; \
			if (v < vMin.c) vMin.c = v; \
			v += 2 * js; \
			if (v > vMax.c) vMax.c = v;

		test(x)
		test(y)
		test(z)
#undef test
	}
}
//-----------------------------------------------------------------------------
void PhysicPrimitiveBody::GetFastBSphere(glm::vec3& center, float& radius)
{
	glm::vec3 b;

	GetAABB(center, b);
	center.x = (center.x + b.x) / 2;
	center.y = (center.y + b.y) / 2;
	center.z = (center.z + b.z) / 2;

	radius = DISTANCE(center, b);
}
//-----------------------------------------------------------------------------
void PhysicPrimitiveBody::MoveTo(glm::vec3 position)
{
	position = position - GetCenterOfMass();

	for (uint8_t i = 0; i < jointCount; ++i)
		joints[i].position = joints[i].position + position;
}
//-----------------------------------------------------------------------------
void PhysicPrimitiveBody::MoveBy(const glm::vec3& offset)
{
	for (uint16_t i = 0; i < jointCount; ++i)
		joints[i].position = joints[i].position + offset;
}
//-----------------------------------------------------------------------------
void PhysicWorld::SetSize(const glm::vec3& center, const glm::vec3& size)
{
	m_centerWorld = center;
	m_sizeWorld = size;
}
//-----------------------------------------------------------------------------
void PhysicWorld::SetGravity(const glm::vec3& gravity)
{
	m_gravity = gravity;
}
//-----------------------------------------------------------------------------
void PhysicWorld::AddBody(PhysicPrimitiveBody* body)
{
	m_bodies.push_back(body);
}
//-----------------------------------------------------------------------------
void PhysicWorld::Tick()
{
	for (size_t i = 0; i < m_bodies.size(); i++)
	{
		auto body = m_bodies[i];
		if (body->flags & (BODY_FLAG_DEACTIVATED | BODY_FLAG_DISABLED))
			continue;

		Joint* joint2 = nullptr;
		glm::vec3 origPos = body->joints[0].position;
		
		Joint* joint = body->joints;
		for (uint16_t j = 0; j < body->jointCount; ++j) // apply velocities
		{
			// non-rotating bodies will copy the 1st joint's velocity
			if (body->flags & BODY_FLAG_NONROTATING)
				joint->velocity = body->joints[0].velocity;

			joint->position += joint->velocity;

			joint++;
		}

		Connection* connection = body->connections;

		glm::vec3 aabbMin, aabbMax;
		body->GetAABB(aabbMin, aabbMax);

		body1Index = i;
		body2Index = body1Index;

		uint8_t collided = bodyEnvironmentResolveCollision(body);

		if (body->flags & BODY_FLAG_NONROTATING)
		{
			/* Non-rotating bodies may end up still colliding after environment coll
			resolvement (unlike rotating bodies where each joint is ensured separately
			to not collide). So if still in collision, we try a few more times. If not
			successful, we simply undo any shifts we've done. This should absolutely
			prevent any body escaping out of environment bounds. */

			for (uint8_t i = 0; i < NONROTATING_COLLISION_RESOLVE_ATTEMPTS; ++i)
			{
				if (!collided)
					break;

				collided = bodyEnvironmentResolveCollision(body);
			}

			if (collided && bodyEnvironmentCollide(body))
				body->MoveBy(origPos - body->joints[0].position);
		}
		else // normal, rotating bodies
		{
			float bodyTension = 0.0f;

			for (uint16_t j = 0; j < body->connectionCount; ++j) // joint tension
			{
				joint = &(body->joints[connection->joint1]);
				joint2 = &(body->joints[connection->joint2]);

				glm::vec3 dir = joint2->position - joint->position;

				float tension = connectionTension(LENGTH(dir), connection->length);

				bodyTension += tension > 0 ? tension : -tension;

				if (tension > TENSION_ACCELERATION_THRESHOLD || tension < -1 * TENSION_ACCELERATION_THRESHOLD)
				{
					vec3Normalize(dir);

					if (tension > TENSION_GREATER_ACCELERATION_THRESHOLD || tension < -1 * TENSION_GREATER_ACCELERATION_THRESHOLD)
					{
						// apply twice the acceleration after a second threshold, not so elegant but seems to work :)
						dir.x *= 2;
						dir.y *= 2;
						dir.z *= 2;
					}

					dir.x /= TENSION_ACCELERATION_DIVIDER;
					dir.y /= TENSION_ACCELERATION_DIVIDER;
					dir.z /= TENSION_ACCELERATION_DIVIDER;

					if (tension < 0)
					{
						dir.x *= -1;
						dir.y *= -1;
						dir.z *= -1;
					}

					joint->velocity[0] += dir.x;
					joint->velocity[1] += dir.y;
					joint->velocity[2] += dir.z;

					joint2->velocity[0] -= dir.x;
					joint2->velocity[1] -= dir.y;
					joint2->velocity[2] -= dir.z;
				}

				connection++;
			}

			if (body->connectionCount > 0)
			{
				uint8_t hard = !(body->flags & BODY_FLAG_SOFT);

				if (hard)
				{
					bodyReshape(body);

					bodyTension /= body->connectionCount;

					if (bodyTension > RESHAPE_TENSION_LIMIT)
						for (uint8_t k = 0; k < RESHAPE_ITERATIONS; ++k)
							bodyReshape(body);
				}

				if (!(body->flags & BODY_FLAG_SIMPLE_CONN))
					bodyCancelOutVelocities(body, hard);
			}
		}

		for (uint16_t j = 0; j < m_bodies.size(); ++j)
		{
			if (j > i || (m_bodies[j]->flags & BODY_FLAG_DEACTIVATED))
			{
				// firstly quick-check collision of body AA bounding boxes

				glm::vec3 aabbMin2, aabbMax2;
				m_bodies[j]->GetAABB(aabbMin2, aabbMax2);

				body2Index = j;

				if (checkOverlapAABB(aabbMin, aabbMax, aabbMin2, aabbMax2) &&
					bodiesResolveCollision(body, m_bodies[j]))
				{
					bodyActivate(body);
					body->deactivateCount = LIGHT_DEACTIVATION;

					bodyActivate(m_bodies[j]);
					m_bodies[j]->deactivateCount = LIGHT_DEACTIVATION;
				}
			}
		}

		if (!(body->flags & BODY_FLAG_ALWAYS_ACTIVE))
		{
			if (body->deactivateCount >= DEACTIVATE_AFTER)
			{
				bodyStop(body);
				body->deactivateCount = 0;
				body->flags |= BODY_FLAG_DEACTIVATED;
			}
			else if (bodyGetAverageSpeed(body) <= LOW_SPEED)
				body->deactivateCount++;
			else
				body->deactivateCount = 0;
		}


		// Apply Gravity
		if ((body->flags & BODY_FLAG_DEACTIVATED) || (body->flags & BODY_FLAG_DISABLED))
			continue;

		for (uint16_t jn = 0; jn < body->jointCount; ++jn)
		{
			body->joints[jn].velocity.x += m_gravity.x;
			body->joints[jn].velocity.y += m_gravity.y;
			body->joints[jn].velocity.z += m_gravity.z;
		}
	}
}
//-----------------------------------------------------------------------------
glm::vec3 PhysicWorld::aaboxInside(glm::vec3 point, float maxDistance)
{
	glm::vec3 center = m_centerWorld;
	glm::vec3 size = m_sizeWorld / 2.0f;
	glm::vec3 shifted = point - center;

	glm::vec3 a = size - shifted;
	glm::vec3 b = shifted + size;

	float sx = 1, sy = 1, sz = 1;

	if (b.x < a.x)
	{
		a.x = b.x;
		sx = -1;
	}

	if (b.y < a.y)
	{
		a.y = b.y;
		sy = -1;
	}

	if (b.z < a.z)
	{
		a.z = b.z;
		sz = -1;
	}

	if (a.x < 0 || a.y < 0 || a.z < 0)
		return point;

	if (a.x < a.y)
	{
		if (a.x < a.z) point.x = center.x + sx * size.x;
		else point.z = center.z + sz * size.z;
	}
	else
	{
		if (a.y < a.z) point.y = center.y + sy * size.y;
		else point.z = center.z + sz * size.z;
	}

	return point;
}
//-----------------------------------------------------------------------------
bool PhysicWorld::bodyEnvironmentResolveCollision(PhysicPrimitiveBody* body)
{
	glm::vec3 c;
	float d;
	body->GetFastBSphere(c, d);

	if (DISTANCE(c, environmentDistance(c, d)) > d)
		return false;

	// now test the full body collision:

	uint8_t collision = 0;

	for (uint16_t i = 0; i < body->jointCount; ++i)
	{
		glm::vec3 previousPos = body->joints[i].position;

		joint1Index = i;

		uint8_t r = jointEnvironmentResolveCollision(body->joints + i, body->elasticity, body->friction);
		if (r)
		{
			collision = 1;

			if (body->flags & BODY_FLAG_NONROTATING)
				bodyNonrotatingJointCollided(body, i, previousPos, r == 1);
		}
	}

	return collision;
}
//-----------------------------------------------------------------------------
uint8_t PhysicWorld::jointEnvironmentResolveCollision(Joint* joint, float elasticity, float friction)
{
	glm::vec3 toJoint = joint->position - environmentDistance(joint->position, JOINT_SIZE(*joint));

	float len = LENGTH(toJoint);

	if (len <= JOINT_SIZE(*joint))
	{
		// TODO: callback
		//if (collisionCallback != 0)
		//	if (!collisionCallback(body1Index, joint1Index, body2Index, joint2Index, (joint->position - toJoint)))
		//		return 0;

		// colliding

		glm::vec3 positionBackup = joint->position, shift;
		uint8_t success = 0;

		if (len > 0)
		{
			// Joint center is still outside the geometry so we can determine the normal and use it to shift it outside. This can still leave the joint colliding though, so try to repeat it a few times.
			for (int i = 0; i < COLLISION_RESOLUTION_ITERATIONS; ++i)
			{
				shift = toJoint;
				vec3Normalize(shift);

				shift = vec3Times(shift, JOINT_SIZE(*joint) - len + COLLISION_RESOLUTION_MARGIN);

				joint->position = joint->position + shift;

				toJoint = joint->position - environmentDistance(joint->position, JOINT_SIZE(*joint));

				len = LENGTH(toJoint); // still colliding?

				if (len >= JOINT_SIZE(*joint))
				{
					success = 1;
					break;
				}
			}
		}

		if (!success)
		{
			// Shifting along normal was unsuccessfull, now try different approach: shift back by joint velocity.

			shift = glm::vec3(-1 * joint->velocity.x, -1 * joint->velocity.y, -1 * joint->velocity.z);

			for (int i = 0; i < COLLISION_RESOLUTION_ITERATIONS; ++i)
			{
				joint->position = joint->position + shift;

				toJoint = joint->position - environmentDistance(joint->position, JOINT_SIZE(*joint));

				len = LENGTH(toJoint); // still colliding?

				if (len >= JOINT_SIZE(*joint))
				{
					success = 1;
					break;
				}

				shift /= 2.0f; // decrease the step a bit
			}
		}

		if (success)
		{
			glm::vec3 vel = joint->velocity;

			vel = vec3Project(vel, shift); // parallel part of velocity

			// perpendicular part of velocity
			glm::vec3 vel2 = joint->velocity - vel;

			vel2 = vec3Times(vel2, friction);

			vel = vec3Times(vel, FRACTIONS_PER_UNIT + elasticity);

			joint->velocity.x -= vel.x + vel2.x;
			joint->velocity.y -= vel.y + vel2.y;
			joint->velocity.z -= vel.z + vel2.z;
		}
		else
		{
			LogWarning("joint-environment collision couldn't be resolved");

			joint->position = positionBackup;
			joint->velocity.x = 0.0f;
			joint->velocity.y = 0.0f;
			joint->velocity.z = 0.0f;

			return 2;
		}

		return 1;
	}

	return 0;
}
//-----------------------------------------------------------------------------
void PhysicWorld::bodyNonrotatingJointCollided(PhysicPrimitiveBody* b, int16_t jointIndex, glm::vec3 origPos, uint8_t success)
{
	origPos = b->joints[jointIndex].position - origPos;

	for (uint16_t i = 0; i < b->jointCount; ++i)
	{
		if (i != jointIndex)
		{
			b->joints[i].position = b->joints[i].position + origPos;

			if (success)
				b->joints[i].velocity = b->joints[jointIndex].velocity;
		}
	}
}
//-----------------------------------------------------------------------------
uint8_t PhysicWorld::bodyEnvironmentCollide(const PhysicPrimitiveBody* body)
{
	for (uint16_t i = 0; i < body->jointCount; ++i)
	{
		const Joint* joint = body->joints + i;

		float size = JOINT_SIZE(*joint);

		if (DISTANCE(joint->position, environmentDistance(joint->position, size)) <= size)
			return 1;
	}

	return 0;
}
//-----------------------------------------------------------------------------
void PhysicWorld::bodyReshape(PhysicPrimitiveBody* body)
{
	for (uint16_t i = 0; i < body->connectionCount; ++i)
	{
		Connection* c = &body->connections[i];

		Joint* j1 = &(body->joints[c->joint1]);
		Joint* j2 = &(body->joints[c->joint2]);

		glm::vec3 dir = j2->position - j1->position;

		glm::vec3 middle = j1->position + j2->position;

		middle.x /= 2;
		middle.y /= 2;
		middle.z /= 2;

		vec3Normalize(dir);

		dir.x = (dir.x * c->length) / FRACTIONS_PER_UNIT;
		dir.y = (dir.y * c->length) / FRACTIONS_PER_UNIT;
		dir.z = (dir.z * c->length) / FRACTIONS_PER_UNIT;

		glm::vec3 positionBackup = j1->position;

		j1->position.x = middle.x - dir.x / 2;
		j1->position.y = middle.y - dir.y / 2;
		j1->position.z = middle.z - dir.z / 2;

		if (LENGTH((j1->position - environmentDistance(j1->position, JOINT_SIZE(*j1)))) < JOINT_SIZE(*j1))
			j1->position = positionBackup;

		positionBackup = j2->position;

		j2->position.x = j1->position.x + dir.x;
		j2->position.y = j1->position.y + dir.y;
		j2->position.z = j1->position.z + dir.z;

		if (LENGTH((j2->position - environmentDistance(j2->position, JOINT_SIZE(*j2)))) < JOINT_SIZE(*j2))
			j2->position = positionBackup;
	}
}
//-----------------------------------------------------------------------------
void PhysicWorld::bodyCancelOutVelocities(PhysicPrimitiveBody* body, uint8_t strong)
{
	for (uint16_t i = 0; i < body->connectionCount; ++i)
	{
		Connection* c = &body->connections[i];

		Joint* j1 = &(body->joints[c->joint1]);
		Joint* j2 = &(body->joints[c->joint2]);

		glm::vec3 dir = j2->position - j1->position;

		float len = nonZero(LENGTH(dir));

		uint8_t cancel = 1;

		if (strong)
		{
			float tension = connectionTension(len, c->length);

			cancel = tension <= TENSION_ACCELERATION_THRESHOLD &&
				tension >= -1 * TENSION_ACCELERATION_THRESHOLD;
		}

		if (cancel)
		{
			glm::vec3
				v1 = j1->velocity,
				v2 = j2->velocity;

			dir.x = (dir.x * FRACTIONS_PER_UNIT) / len; // normalize
			dir.y = (dir.y * FRACTIONS_PER_UNIT) / len;
			dir.z = (dir.z * FRACTIONS_PER_UNIT) / len;

			v1 = vec3ProjectNormalized(v1, dir);
			v2 = vec3ProjectNormalized(v2, dir);

			glm::vec avg = v1 + v2;

			avg.x /= 2;
			avg.y /= 2;
			avg.z /= 2;

			if (strong)
			{
				j1->velocity.x = j1->velocity.x - v1.x + avg.x;
				j1->velocity.y = j1->velocity.y - v1.y + avg.y;
				j1->velocity.z = j1->velocity.z - v1.z + avg.z;

				j2->velocity.x = j2->velocity.x - v2.x + avg.x;
				j2->velocity.y = j2->velocity.y - v2.y + avg.y;
				j2->velocity.z = j2->velocity.z - v2.z + avg.z;
			}
			else
			{
				j1->velocity.x = j1->velocity.x - v1.x + (v1.x * 3 + avg.x) / 4;
				j1->velocity.y = j1->velocity.y - v1.y + (v1.y * 3 + avg.y) / 4;
				j1->velocity.z = j1->velocity.z - v1.z + (v1.z * 3 + avg.z) / 4;

				j2->velocity.x = j2->velocity.x - v2.x + (v2.x * 3 + avg.x) / 4;
				j2->velocity.y = j2->velocity.y - v2.y + (v2.y * 3 + avg.y) / 4;
				j2->velocity.z = j2->velocity.z - v2.z + (v2.z * 3 + avg.z) / 4;
			}
		}
	}
}
//-----------------------------------------------------------------------------
uint8_t PhysicWorld::bodiesResolveCollision(PhysicPrimitiveBody* b1, PhysicPrimitiveBody* b2)
{
	uint8_t r = 0;

	for (uint16_t i = 0; i < b1->jointCount; ++i)
	{
		for (uint16_t j = 0; j < b2->jointCount; ++j)
		{
			glm::vec3 origPos2 = b2->joints[j].position;
			glm::vec3 origPos1 = b1->joints[i].position;

			joint1Index = i;
			joint2Index = j;

			if (jointsResolveCollision(&(b1->joints[i]), &(b2->joints[j]),
				b1->jointMass, b2->jointMass, (b1->elasticity + b2->elasticity) / 2,
				(b1->friction + b2->friction) / 2))
			{
				r = 1;

				if (b1->flags & BODY_FLAG_NONROTATING)
					bodyNonrotatingJointCollided(b1, i, origPos1, 1);

				if (b2->flags & BODY_FLAG_NONROTATING)
					bodyNonrotatingJointCollided(b2, j, origPos2, 1);
			}
		}
	}

	return r;
}
//-----------------------------------------------------------------------------
uint8_t PhysicWorld::jointsResolveCollision(Joint* j1, Joint* j2, float mass1, float mass2, float elasticity, float friction)
{
	glm::vec3 dir = j2->position - j1->position;

	float d = LENGTH(dir) - JOINT_SIZE(*j1) - JOINT_SIZE(*j2);

	if (d < 0) // collision?
	{
		// TODO: callback
		//if (collisionCallback != 0 && !collisionCallback(body1Index, joint1Index, body2Index, joint2Index, (j1->position + dir)))
		//	return 0;

		glm::vec3
			pos1Backup = j1->position,
			pos2Backup = j2->position;

		// separate joints, the shift distance will depend on the weight ratio:

		d = -1 * d + COLLISION_RESOLUTION_MARGIN;

		vec3Normalize(dir);

		float ratio = (mass2 * FRACTIONS_PER_UNIT) / nonZero(mass1 + mass2);

		float shiftDistance = (ratio * d) / FRACTIONS_PER_UNIT;

		glm::vec3 shift = vec3Times(dir, shiftDistance);

		j1->position = j1->position - shift;

		shiftDistance = d - shiftDistance;

		shift = vec3Times(dir, shiftDistance);

		j2->position = j2->position + shift;

		// compute new velocities:

		float v1, v2;

		glm::vec3 vel = j1->velocity;

		vel = vec3Project(vel, dir);

		j1->velocity.x = j1->velocity.x - vel.x;
		j1->velocity.y = j1->velocity.y - vel.y;
		j1->velocity.z = j1->velocity.z - vel.z;

		/* friction explanation: Not physically correct (doesn't depend on load),
		friction basically means we weighted average the velocities of the bodies
		in the direction perpendicular to the hit normal, in the ratio of their
		masses, friction coefficient just says how much of this effect we apply
		(it multiplies the friction vectors we are subtracting) */

		glm::vec3 frictionVec = j1->velocity;

		v1 = vec3Dot(vel, dir);
		vel = j2->velocity;
		vel = vec3Project(vel, dir);

		j2->velocity[0] = j2->velocity[0] - vel.x;
		j2->velocity[1] = j2->velocity[1] - vel.y;
		j2->velocity[2] = j2->velocity[2] - vel.z;

		frictionVec = j2->velocity - frictionVec;

		v2 = vec3Dot(vel, dir);

		getVelocitiesAfterCollision(&v1, &v2, mass1, mass2, elasticity);

		vel = vec3Times(dir, v1);

#define assignVec(_n,j,i,d,o) \
  j->velocity._n = j->velocity._n + vel.d o (((frictionVec.d * ratio) / FRACTIONS_PER_UNIT) * friction) / FRACTIONS_PER_UNIT;

		assignVec(x, j1, 0, x, +)
		assignVec(y, j1, 1, y, +)
		assignVec(z, j1, 2, z, +)

		vel = vec3Times(dir, v2);

		ratio = FRACTIONS_PER_UNIT - ratio;

		assignVec(x, j2, 0, x, -)
		assignVec(y, j2, 1, y, -)
		assignVec(z, j2, 2, z, -)

#undef assignVec

		// ensure the joints aren't colliding with environment

		if (jointEnvironmentResolveCollision(j1, elasticity, friction) == 2)
			j1->position = pos1Backup;

		if (jointEnvironmentResolveCollision(j2, elasticity, friction) == 2)
			j2->position = pos2Backup;

		return 1;
	}

	return 0;
}
//-----------------------------------------------------------------------------
void PhysicWorld::bodyActivate(PhysicPrimitiveBody* body)
{
	// the if check has to be here, don't remove it
	if (body->flags & BODY_FLAG_DEACTIVATED)
	{
		bodyStop(body);
		body->flags &= ~BODY_FLAG_DEACTIVATED;
		body->deactivateCount = 0;
	}
}
//-----------------------------------------------------------------------------
void PhysicWorld::bodyStop(PhysicPrimitiveBody* body)
{
	for (uint16_t i = 0; i < body->jointCount; ++i)
	{
		body->joints[i].velocity.x = 0;
		body->joints[i].velocity.y = 0;
		body->joints[i].velocity.z = 0;
	}
}
//-----------------------------------------------------------------------------
float PhysicWorld::bodyGetAverageSpeed(const PhysicPrimitiveBody* body)
{
	return bodyGetNetSpeed(body) / body->jointCount;
}
//-----------------------------------------------------------------------------
float PhysicWorld::bodyGetNetSpeed(const PhysicPrimitiveBody* body)
{
#if APPROXIMATE_NET_SPEED
	glm::vec3 netV = glm::vec3(0, 0, 0);

	const Joint* joint = body->joints;

	for (uint16_t i = 0; i < body->jointCount; ++i)
	{
		netV.x += _abs(joint->velocity.x);
		netV.y += _abs(joint->velocity.y);
		netV.z += _abs(joint->velocity.z);

		joint++;
	}

	return vec3LenApprox(netV);
#else
	float velocity = 0;

	const Joint* joint = body->joints;

	for (uint16_t i = 0; i < body->jointCount; ++i)
	{
		velocity += LENGTH(joint->velocity);
		joint++;
	}

	return velocity;
#endif
}
//-----------------------------------------------------------------------------
#endif // USE_MICROPHYS
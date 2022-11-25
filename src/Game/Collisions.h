#pragma once

#include "BaseHeader.h"
#include "EngineMath.h"

namespace Collisions
{
	struct HitInfo
	{
		glm::vec3 point = glm::vec3(0.0);
		glm::vec3 normal = glm::vec3(0.0); // TODO: rename?
		float length = HUGE_VALF;
		bool isCollision = false;
	};

	struct ClosestHitInfo
	{
		bool IsValid() const { return length < HUGE_VALF; }

		glm::vec3 point = glm::vec3(0.0);
		glm::vec3 normal = glm::vec3(0.0); // TODO: rename?
		float length = HUGE_VALF;
	};


	// finds the closest point to the source point on the given line segment
	inline constexpr glm::vec3 ClosestPointOnLineSegment(
		const glm::vec3& a, // point one of line segment
		const glm::vec3& b, // point two of line segment
		const glm::vec3& point  // source point
	);
	inline constexpr glm::vec3 ClosestPointOnLineSegment(const Line& line, const glm::vec3& point);

	// finds the closest point on the triangle from the source point given
	// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
	inline ClosestHitInfo ClosestPointOnTriangle(
		const glm::vec3& tri0,
		const glm::vec3& tri1,
		const glm::vec3& tri2,
		const glm::vec3& triNormal,
		const glm::vec3& point);

	inline HitInfo TriangleOnSphere(
		// triangle
		const glm::vec3& tri0,
		const glm::vec3& tri1,
		const glm::vec3& tri2,
		const glm::vec3& triNormal,
		// sphere
		const glm::vec3& point,
		float radius
	);

	// finds the collision point between a triangle and a capsule
	// capsules are defined with two pointsand a radius
	// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
	inline HitInfo TriangleOnCapsule(
		// triangle
		const glm::vec3& tri0,
		const glm::vec3& tri1,
		const glm::vec3& tri2,
		const glm::vec3& triNormal,
		// capsule
		const glm::vec3& tip,
		const glm::vec3& base,
		const glm::vec3& a,
		const glm::vec3& b,
		const glm::vec3& capn,
		float radius
	);

	inline ClosestHitInfo TriangleCapsuleFindClosest(const Poly& poly,
		const glm::vec3& tip,
		const glm::vec3& base,
		const glm::vec3& a,
		const glm::vec3& b,
		const glm::vec3& norm,
		float radius);

	inline ClosestHitInfo CapsuleIntersection(const Poly& poly,
		const glm::vec3& tip,
		const glm::vec3& base,
		float radius);
}

#include "Collisions.inl"
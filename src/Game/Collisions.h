#pragma once

#include "BaseHeader.h"
#include "EngineMath.h"

namespace Collisions
{
	// finds the closest point to the source point on the given line segment
	inline constexpr glm::vec3 ClosestPointOnLineSegment(
		const glm::vec3& a, // point one of line segment
		const glm::vec3& b, // point two of line segment
		const glm::vec3& point  // source point
	);
	inline constexpr glm::vec3 ClosestPointOnLineSegment(const Line& line, const glm::vec3& point);

	inline bool CheckPointInTriangle(const glm::vec3& tri0, const glm::vec3& tri1, const glm::vec3& tri2, const glm::vec3& point);

	inline bool GetLowestRoot(float a, float b, float c, float maxR, float* root);

	// Möller–Trumbore intersection algorithm
	inline int RayInTri(const glm::vec3& from, const glm::vec3& to, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& intersect);
}

#include "Collisions.inl"
#include "Collisions2.h"
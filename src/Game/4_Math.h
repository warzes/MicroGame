#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

struct Plane
{
	Plane() = default;
	Plane(const glm::vec3& normal, const glm::vec3& det)
	{
		n = normal;
		d = det;
	}
	Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		d = a;
		n = glm::normalize(glm::cross(-b + a, c - a));
	}

	glm::vec3 n = glm::vec3(0, 1, 0);
	glm::vec3 d = glm::vec3(0, 0, 0);
};

struct Sphere
{
	Sphere() = default;
	Sphere(const glm::vec3& position, float size)
	{
		pos = position;
		radius = size;
	}
	glm::vec3 pos = glm::vec3(0, 0, 0);
	float radius = 1.0f;
};

enum class VolumeCheck
{
	OUTSIDE,
	INTERSECT,
	CONTAINS
};

struct FrustumCorners
{
	// Utility to transform frustum into any objects space
	// Useful for complex frustum culling operations
	void Transform(glm::mat4 space);
	// near plane
	glm::vec3 na;
	glm::vec3 nb;
	glm::vec3 nc;
	glm::vec3 nd;
	// far plane
	glm::vec3 fa;
	glm::vec3 fb;
	glm::vec3 fc;
	glm::vec3 fd;
};

class Frustum
{
public:
	void Update(float aspectRatio);

	void Set(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& right, float nearPlane, float farPlane, float FOV);

	void SetCullTransform(glm::mat4 objectWorld);

	VolumeCheck ContainsPoint(const glm::vec3& point) const;
	VolumeCheck ContainsSphere(const Sphere& sphere) const;
	VolumeCheck ContainsTriangle(glm::vec3& a, glm::vec3& b, glm::vec3& c);
	VolumeCheck ContainsTriVolume(glm::vec3& a, glm::vec3& b, glm::vec3& c, float height);

	const glm::vec3& GetPositionOS() const { return m_positionObject; }
	const float GetFOV() const { return m_FOV; }
	const float GetRadInvFOV() const { return m_radInvFOV; }

	FrustumCorners GetCorners() const { return m_corners; }

private:
	//transform to the culled objects object space and back to world space
	glm::mat4 m_cullWorld, m_cullInverse;

	//stuff in the culled objects object space
	std::vector<Plane> m_planes;
	FrustumCorners m_corners = FrustumCorners();
	glm::vec3 m_positionObject;

	float m_radInvFOV;

	//camera parameters for locking
	glm::vec3 m_position;
	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;
	float m_nearPlane, m_farPlane, m_FOV;
};

namespace math
{
	// Returns true if axis-aligned bounding boxes intersect.
	inline bool IntersectAABB(glm::vec3 a_position, glm::vec3 a_size, glm::vec3 b_position, glm::vec3 b_size)
	{
		for (auto i = 0; i < 3; ++i)
		{
			if (std::abs(a_position[i] - b_position[i]) * 2 >= (a_size[i] + b_size[i]))
				return false;
		}
		return true;
	}

	// Returns true if axis-aligned bounding box and ray intersect.
	inline bool IntersectAABBRay(
		const glm::vec3 aabb_position,
		const glm::vec3 aabb_size,
		const glm::vec3 ray_origin,
		const glm::vec3 ray_direction,
		glm::vec3& point)
	{
		glm::vec3 tmin, tmax;
		glm::vec3 bounds[2] = { aabb_position - aabb_size / 2.0f, aabb_position + aabb_size / 2.0f };

		glm::vec3 inverse_direction = 1.0f / ray_direction;

		tmin.x = (bounds[inverse_direction.x < 0].x - ray_origin.x) * inverse_direction.x;
		tmax.x = (bounds[inverse_direction.x >= 0].x - ray_origin.x) * inverse_direction.x;
		tmin.y = (bounds[inverse_direction.y < 0].y - ray_origin.y) * inverse_direction.y;
		tmax.y = (bounds[inverse_direction.y >= 0].y - ray_origin.y) * inverse_direction.y;

		if ((tmin.x > tmax.y) || (tmin.y > tmax.x))
			return false;
		if (tmin.y > tmin.x)
			tmin.x = tmin.y;
		if (tmax.y < tmax.x)
			tmax.x = tmax.y;

		tmin.z = (bounds[inverse_direction.z < 0].z - ray_origin.z) * inverse_direction.z;
		tmax.z = (bounds[inverse_direction.z >= 0].z - ray_origin.z) * inverse_direction.z;

		if ((tmin.x > tmax.z) || (tmin.z > tmax.x))
			return false;
		if (tmin.z > tmin.x)
			tmin.x = tmin.z;
		if (tmax.z < tmax.x)
			tmax.x = tmax.z;

		float t = tmin.x;

		if (t < 0)
		{
			t = tmax.x;
			if (t < 0)
				return false;
		}

		point = ray_origin + ray_direction * t;

		return true;
	}
}
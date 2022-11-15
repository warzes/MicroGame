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

namespace collisions
{
	// --finds the closest point to the source point on the given line segment
		/*--finds the closest point to the source point on the given line segment
		local function closestPointOnLineSegment(
			a_x, a_y, a_z, --point one of line segment
			b_x, b_y, b_z, --point two of line segment
			x, y, z        -- source point
		)
		local ab_x, ab_y, ab_z = b_x - a_x, b_y - a_y, b_z - a_z

		local t = vectorDotProduct(x - a_x, y - a_y, z - a_z, ab_x, ab_y, ab_z) / (ab_x ^ 2 + ab_y ^ 2 + ab_z ^ 2)
		t = math.min(1, math.max(0, t))
		return a_x + t * ab_x, a_y + t * ab_y, a_z + t * ab_z
		end*/
	inline glm::vec3 ClosestPointOnLineSegment(
		float a_x, float a_y, float a_z, // --point one of line segment
		float b_x, float b_y, float b_z, // --point two of line segment
		float x, float y, float z) // --source point
	{
		// todo: in glm::vec3
		float ab_x = b_x - a_x;
		float ab_y = b_y - a_y;
		float ab_z = b_z - a_z;

		float t = glm::dot(glm::vec3{ x - a_x, y - a_y, z - a_z }, glm::vec3{ ab_x, ab_y, ab_z }) / (ab_x * ab_x + ab_y * ab_y + ab_z * ab_z);
		t = std::min(1.0f, std::max(0.0f, t));
		return glm::vec3{
			a_x + t * ab_x,
			a_y + t * ab_y,
			a_z + t * ab_z
		};
	}

	/*
	-- model - ray intersection
	-- based off of triangle - ray collision from excessive's CPML library
	-- does a triangle - ray collision for every face in the model to find the shortest collision
	--
	-- sources:
	--     https://github.com/excessive/cpml/blob/master/modules/intersect.lua
	--     http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
	local tiny = 2.2204460492503131e-16 -- the smallest possible value for a double, "double epsilon"
	local function triangleRay(
			tri_0_x, tri_0_y, tri_0_z,
			tri_1_x, tri_1_y, tri_1_z,
			tri_2_x, tri_2_y, tri_2_z,
			n_x, n_y, n_z,
			src_x, src_y, src_z,
			dir_x, dir_y, dir_z
		)

		-- cache these variables for efficiency
		local e11,e12,e13 = fastSubtract(tri_1_x,tri_1_y,tri_1_z, tri_0_x,tri_0_y,tri_0_z)
		local e21,e22,e23 = fastSubtract(tri_2_x,tri_2_y,tri_2_z, tri_0_x,tri_0_y,tri_0_z)
		local h1,h2,h3 = vectorCrossProduct(dir_x,dir_y,dir_z, e21,e22,e23)
		local a = vectorDotProduct(h1,h2,h3, e11,e12,e13)

		-- if a is too close to 0, ray does not intersect triangle
		if math.abs(a) <= tiny then
			return
		end

		local s1,s2,s3 = fastSubtract(src_x,src_y,src_z, tri_0_x,tri_0_y,tri_0_z)
		local u = vectorDotProduct(s1,s2,s3, h1,h2,h3) / a

		-- ray does not intersect triangle
		if u < 0 or u > 1 then
			return
		end

		local q1,q2,q3 = vectorCrossProduct(s1,s2,s3, e11,e12,e13)
		local v = vectorDotProduct(dir_x,dir_y,dir_z, q1,q2,q3) / a

		-- ray does not intersect triangle
		if v < 0 or u + v > 1 then
			return
		end

		-- at this stage we can compute t to find out where
		-- the intersection point is on the line
		local thisLength = vectorDotProduct(q1,q2,q3, e21,e22,e23) / a

		-- if hit this triangle and it's closer than any other hit triangle
		if thisLength >= tiny and (not finalLength or thisLength < finalLength) then
			--local norm_x, norm_y, norm_z = vectorCrossProduct(e11,e12,e13, e21,e22,e23)

			return thisLength, src_x + dir_x*thisLength, src_y + dir_y*thisLength, src_z + dir_z*thisLength, n_x,n_y,n_z
		end
	end
	
	*/

	struct RetTriangleRay
	{
		float Length = 0.0f;
		glm::vec3 dir;
		glm::vec3 n;
	};

	inline RetTriangleRay TriangleRay(
		float tri_0_x, float tri_0_y, float tri_0_z,
		float tri_1_x, float tri_1_y, float tri_1_z,
		float tri_2_x, float tri_2_y, float tri_2_z,
		float n_x,     float n_y,     float n_z,
		float src_x,   float src_y,   float src_z,
		float dir_x,   float dir_y,   float dir_z,
		float finalLength
	)
	{
		constexpr float tiny = 2.2204460492503131e-16; // --the smallest possible value for a double, "double epsilon"

		// --cache these variables for efficiency
		float e11 = tri_1_x - tri_0_x;
		float e12 = tri_1_y - tri_0_y;
		float e13 = tri_1_z - tri_0_z;

		float e21 = tri_2_x - tri_0_x;
		float e22 = tri_2_y - tri_0_y;
		float e23 = tri_2_z - tri_0_z;

		glm::vec3 h = glm::cross(glm::vec3{ dir_x, dir_y, dir_z }, glm::vec3{ e21, e22, e23 });

		float a = glm::dot(h, glm::vec3{ e11, e12, e13 });

		// -- if a is too close to 0, ray does not intersect triangle
		if (abs(a) <= tiny)
			return {};

		float s1 = src_x - tri_0_x;
		float s2 = src_y - tri_0_y;
		float s3 = src_z - tri_0_z;

		float u = glm::dot(glm::vec3{s1, s2, s3 }, h) / a;

		// -- ray does not intersect triangle
		if (u < 0.0f || u > 1.0f)
			return {};

		glm::vec3 q = glm::cross(glm::vec3{ s1, s2, s3 }, glm::vec3{ e11, e12, e13 });

		float v = glm::dot(glm::vec3{ dir_x, dir_y, dir_z }, q) / a;
		// -- ray does not intersect triangle
		if (v < 0.0f || u +v > 1.0f)
			return {};

		// --at this stage we can compute t to find out where
		// -- the intersection point is on the line
		float thisLength = glm::dot(q, { e21, e22, e23 }) / a;

		// -- if hit this triangleand it's closer than any other hit triangle
		if (thisLength >= tiny && finalLength != 0.0f && thisLength < finalLength)
		{
			// --local norm_x, norm_y, norm_z = vectorCrossProduct(e11, e12, e13, e21, e22, e23)
			return {
				thisLength,
				glm::vec3(src_x + dir_x * thisLength, src_y + dir_y * thisLength, src_z + dir_z * thisLength),
				glm::vec3(n_x, n_y, n_z),
			};
		}

		return {};
	}

	/*
	-- detects a collision between a triangle and a sphere
--
-- sources:
--     https://wickedengine.net/2020/04/26/capsule-collision-detection/
local function triangleSphere(
        tri_0_x, tri_0_y, tri_0_z,
        tri_1_x, tri_1_y, tri_1_z,
        tri_2_x, tri_2_y, tri_2_z,
        tri_n_x, tri_n_y, tri_n_z,
        src_x, src_y, src_z, radius
    )

    -- recalculate surface normal of this triangle
    local side1_x, side1_y, side1_z = tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z
    local side2_x, side2_y, side2_z = tri_2_x - tri_0_x, tri_2_y - tri_0_y, tri_2_z - tri_0_z
    local n_x, n_y, n_z = vectorNormalize(vectorCrossProduct(side1_x, side1_y, side1_z, side2_x, side2_y, side2_z))

    -- distance from src to a vertex on the triangle
    local dist = vectorDotProduct(src_x - tri_0_x, src_y - tri_0_y, src_z - tri_0_z, n_x, n_y, n_z)

    -- collision not possible, just return
    if dist < -radius or dist > radius then
        return
    end

    -- itx stands for intersection
    local itx_x, itx_y, itx_z = src_x - n_x * dist, src_y - n_y * dist, src_z - n_z * dist

    -- determine whether itx is inside the triangle
    -- project it onto the triangle and return if this is the case
    local c0_x, c0_y, c0_z = vectorCrossProduct(itx_x - tri_0_x, itx_y - tri_0_y, itx_z - tri_0_z, tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z)
    local c1_x, c1_y, c1_z = vectorCrossProduct(itx_x - tri_1_x, itx_y - tri_1_y, itx_z - tri_1_z, tri_2_x - tri_1_x, tri_2_y - tri_1_y, tri_2_z - tri_1_z)
    local c2_x, c2_y, c2_z = vectorCrossProduct(itx_x - tri_2_x, itx_y - tri_2_y, itx_z - tri_2_z, tri_0_x - tri_2_x, tri_0_y - tri_2_y, tri_0_z - tri_2_z)
    if  vectorDotProduct(c0_x, c0_y, c0_z, n_x, n_y, n_z) <= 0
    and vectorDotProduct(c1_x, c1_y, c1_z, n_x, n_y, n_z) <= 0
    and vectorDotProduct(c2_x, c2_y, c2_z, n_x, n_y, n_z) <= 0 then
        n_x, n_y, n_z = src_x - itx_x, src_y - itx_y, src_z - itx_z

        -- the sphere is inside the triangle, so the normal is zero
        -- instead, just return the triangle's normal
        if n_x == 0 and n_y == 0 and n_z == 0 then
            return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, tri_n_x, tri_n_y, tri_n_z
        end

        return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, n_x, n_y, n_z
    end

    -- itx is outside triangle
    -- find points on all three line segments that are closest to itx
    -- if distance between itx and one of these three closest points is in range, there is an intersection
    local radiussq = radius * radius
    local smallestDist

    local line1_x, line1_y, line1_z = closestPointOnLineSegment(tri_0_x, tri_0_y, tri_0_z, tri_1_x, tri_1_y, tri_1_z, src_x, src_y, src_z)
    local dist = (src_x - line1_x)^2 + (src_y - line1_y)^2 + (src_z - line1_z)^2
    if dist <= radiussq then
        smallestDist = dist
        itx_x, itx_y, itx_z = line1_x, line1_y, line1_z
    end

    local line2_x, line2_y, line2_z = closestPointOnLineSegment(tri_1_x, tri_1_y, tri_1_z, tri_2_x, tri_2_y, tri_2_z, src_x, src_y, src_z)
    local dist = (src_x - line2_x)^2 + (src_y - line2_y)^2 + (src_z - line2_z)^2
    if (smallestDist and dist < smallestDist or not smallestDist) and dist <= radiussq then
        smallestDist = dist
        itx_x, itx_y, itx_z = line2_x, line2_y, line2_z
    end

    local line3_x, line3_y, line3_z = closestPointOnLineSegment(tri_2_x, tri_2_y, tri_2_z, tri_0_x, tri_0_y, tri_0_z, src_x, src_y, src_z)
    local dist = (src_x - line3_x)^2 + (src_y - line3_y)^2 + (src_z - line3_z)^2
    if (smallestDist and dist < smallestDist or not smallestDist) and dist <= radiussq then
        smallestDist = dist
        itx_x, itx_y, itx_z = line3_x, line3_y, line3_z
    end

    if smallestDist then
        n_x, n_y, n_z = src_x - itx_x, src_y - itx_y, src_z - itx_z

        -- the sphere is inside the triangle, so the normal is zero
        -- instead, just return the triangle's normal
        if n_x == 0 and n_y == 0 and n_z == 0 then
            return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, tri_n_x, tri_n_y, tri_n_z
        end

        return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, n_x, n_y, n_z
    end
end
	*/
	// -- detects a collision between a triangle and a sphere
	// -- sources:https://wickedengine.net/2020/04/26/capsule-collision-detection/

	inline float TriangleSphere(
		float tri_0_x, float tri_0_y, float tri_0_z,
		float tri_1_x, float tri_1_y, float tri_1_z,
		float tri_2_x, float tri_2_y, float tri_2_z,
		float tri_n_x, float tri_n_y, float tri_n_z,
		float src_x,   float src_y,   float src_z, 
		float radius
	)
	{
		// -- recalculate surface normal of this triangle
		float side1_x = tri_1_x - tri_0_x;
		float side1_y = tri_1_y - tri_0_y;
		float side1_z = tri_1_z - tri_0_z;

		float side2_x = tri_2_x - tri_0_x;
		float side2_y = tri_2_y - tri_0_y;
		float side2_z = tri_2_z - tri_0_z;

		glm::vec3 n = glm::normalize(glm::cross(glm::vec3(side1_x, side1_y, side1_z), glm::vec3(side2_x, side2_y, side2_z)));

		// -- distance from src to a vertex on the triangle
		float dist = glm::dot(glm::vec3(src_x - tri_0_x, src_y - tri_0_y, src_z - tri_0_z), n);
		// -- collision not possible, just return
		if (dist < -radius || dist > radius)
			return {};

		// -- itx stands for intersection
		float itx_x = src_x - n.x * dist;
		float itx_y = src_y - n.y * dist;
		float itx_z = src_z - n.z * dist;

		//-- determine whether itx is inside the triangle
		//-- project it onto the triangle and return if this is the case
		glm::vec3 c0 = glm::cross(glm::vec3(itx_x - tri_0_x, itx_y - tri_0_y, itx_z - tri_0_z), glm::vec3(tri_1_x - tri_0_x, tri_1_y - tri_0_y, tri_1_z - tri_0_z));
		glm::vec3 c1 = glm::cross(glm::vec3(itx_x - tri_1_x, itx_y - tri_1_y, itx_z - tri_1_z), glm::vec3(tri_2_x - tri_1_x, tri_2_y - tri_1_y, tri_2_z - tri_1_z));
		glm::vec3 c2 = glm::cross(glm::vec3(itx_x - tri_2_x, itx_y - tri_2_y, itx_z - tri_2_z), glm::vec3(tri_0_x - tri_2_x, tri_0_y - tri_2_y, tri_0_z - tri_2_z));

		if (glm::dot(c0, n) <= 0.0f &&
			glm::dot(c1, n) <= 0.0f &&
			glm::dot(c2, n) <= 0.0f
			)
		{
			n.x = src_x - itx_x;
			n.y = src_y - itx_y;
			n.z = src_z - itx_z;
			//--the sphere is inside the triangle, so the normal is zero
			//-- instead, just return the triangle's normal
			if (n.x == 0 && n.y == 0 && n.z == 0)
			{
				return vectorMagnitude(n), itx_x, itx_y, itx_z, tri_n_x, tri_n_y, tri_n_z
			}

			return vectorMagnitude(n), itx_x, itx_y, itx_z, n_x, n_y, n_z
		}

		//--itx is outside triangle
		//-- find points on all three line segments that are closest to itx
		//-- if distance between itxand one of these three closest points is in range, there is an intersection
		float radiussq = radius * radius;

	}

	/*	
	local smallestDist

	local line1_x, line1_y, line1_z = closestPointOnLineSegment(tri_0_x, tri_0_y, tri_0_z, tri_1_x, tri_1_y, tri_1_z, src_x, src_y, src_z)
	local dist = (src_x - line1_x)^2 + (src_y - line1_y)^2 + (src_z - line1_z)^2
	if dist <= radiussq then
		smallestDist = dist
		itx_x, itx_y, itx_z = line1_x, line1_y, line1_z
	end

	local line2_x, line2_y, line2_z = closestPointOnLineSegment(tri_1_x, tri_1_y, tri_1_z, tri_2_x, tri_2_y, tri_2_z, src_x, src_y, src_z)
	local dist = (src_x - line2_x)^2 + (src_y - line2_y)^2 + (src_z - line2_z)^2
	if (smallestDist and dist < smallestDist or not smallestDist) and dist <= radiussq then
		smallestDist = dist
		itx_x, itx_y, itx_z = line2_x, line2_y, line2_z
	end

	local line3_x, line3_y, line3_z = closestPointOnLineSegment(tri_2_x, tri_2_y, tri_2_z, tri_0_x, tri_0_y, tri_0_z, src_x, src_y, src_z)
	local dist = (src_x - line3_x)^2 + (src_y - line3_y)^2 + (src_z - line3_z)^2
	if (smallestDist and dist < smallestDist or not smallestDist) and dist <= radiussq then
		smallestDist = dist
		itx_x, itx_y, itx_z = line3_x, line3_y, line3_z
	end

	if smallestDist then
		n_x, n_y, n_z = src_x - itx_x, src_y - itx_y, src_z - itx_z

		-- the sphere is inside the triangle, so the normal is zero
		-- instead, just return the triangle's normal
		if n_x == 0 and n_y == 0 and n_z == 0 then
			return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, tri_n_x, tri_n_y, tri_n_z
		end

		return vectorMagnitude(n_x, n_y, n_z), itx_x, itx_y, itx_z, n_x, n_y, n_z
	end
end
	*/
}
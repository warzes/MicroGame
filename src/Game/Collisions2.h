#pragma once

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

// finds the closest point on the triangle from the source point given
// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
inline ClosestHitInfo ClosestPointOnTriangle(const glm::vec3& tri0, const glm::vec3& tri1, const glm::vec3& tri2, const glm::vec3& triNormal, const glm::vec3& point)
{
	ClosestHitInfo retInfo;

	// recalculate surface normal of this triangle
	glm::vec3 side1 = tri1 - tri0;
	glm::vec3 side2 = tri2 - tri0;
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	// distance from src to a vertex on the triangle
	float dist = glm::dot((point - tri0), norm);

	// itx stands for intersection
	glm::vec3 itx = point - norm * dist;

	// determine whether itx is inside the triangle project it onto the triangleand return if this is the case
	glm::vec3 c0 = glm::cross((itx - tri0), (tri1 - tri0));
	glm::vec3 c1 = glm::cross((itx - tri1), (tri2 - tri1));
	glm::vec3 c2 = glm::cross((itx - tri2), (tri0 - tri2));

	if (glm::dot(c0, norm) <= 0.0f &&
		glm::dot(c1, norm) <= 0.0f &&
		glm::dot(c2, norm) <= 0.0f)
	{
		norm = point - itx;

		// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
		if (norm.x == 0.0f && norm.y == 0.0f && norm.z == 0.0f) retInfo.normal = triNormal;
		else retInfo.normal = norm;
		retInfo.point = itx;
		retInfo.length = Length(norm);
		return retInfo;
	}

	// itx is outside triangle
	// find points on all three line segments that are closest to itx
	// if distance between itxand one of these three closest points is in range, there is an intersection
	const glm::vec3 line1 = Collisions::ClosestPointOnLineSegment(tri0, tri1, point);
	const glm::vec3 line2 = Collisions::ClosestPointOnLineSegment(tri1, tri2, point);
	const glm::vec3 line3 = Collisions::ClosestPointOnLineSegment(tri2, tri0, point);
	const float mag1 = LengthSq(point - line1);
	const float mag2 = LengthSq(point - line2);
	const float mag3 = LengthSq(point - line3);

	float smallestDist = mag1;
	itx = line1;
	if (mag2 < smallestDist)
	{
		smallestDist = mag2;
		itx = line2;
	}
	if (mag3 < smallestDist)
	{
		smallestDist = mag3;
		itx = line3;
	}

	norm = point - itx;

	// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
	if (norm.x == 0.0f && norm.y == 0.0f && norm.z == 0.0f) retInfo.normal = triNormal;
	else retInfo.normal = norm;
	retInfo.point = itx;
	retInfo.length = Length(norm);
	return retInfo;
}

// detects a collision between a triangleand a sphere
// sources: https://wickedengine.net/2020/04/26/capsule-collision-detection/
// TODO: ������� ����� ��������???
inline HitInfo TriangleOnSphere(const glm::vec3& tri0, const glm::vec3& tri1, const glm::vec3& tri2, const glm::vec3& triNormal, const glm::vec3& point, float radius)
{
	HitInfo retInfo;

	// recalculate surface normal of this triangle
	glm::vec3 side1 = tri1 - tri0;
	glm::vec3 side2 = tri2 - tri0;
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	// distance from src to a vertex on the triangle
	float dist = glm::dot((point - tri0), norm);

	// collision not possible, just return
	if (dist < -radius || dist > radius)
		return {};

	// itx stands for intersection
	glm::vec3 itx = point - norm * dist;

	// determine whether itx is inside the triangle project it onto the triangle and return if this is the case
	glm::vec3 c0 = glm::cross((itx - tri0), (tri1 - tri0));
	glm::vec3 c1 = glm::cross((itx - tri1), (tri2 - tri1));
	glm::vec3 c2 = glm::cross((itx - tri2), (tri0 - tri2));
	if (glm::dot(c0, norm) <= 0.0f &&
		glm::dot(c1, norm) <= 0.0f &&
		glm::dot(c2, norm) <= 0.0f)
	{
		norm = point - itx;

		// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
		if (norm.x == 0.0f && norm.y == 0.0f && norm.z == 0.0f) retInfo.normal = triNormal;
		else retInfo.normal = norm;
		retInfo.point = itx;
		retInfo.length = Length(norm);
		retInfo.isCollision = true; // ???
		return retInfo;
	}

	// itx is outside triangle
	// find points on all three line segments that are closest to itx
	// if distance between itxand one of these three closest points is in range, there is an intersection
	const float radiussq = radius * radius;
	float smallestDist = std::numeric_limits<float>::infinity();

	const glm::vec3 line1 = Collisions::ClosestPointOnLineSegment(tri0, tri1, point);
	const glm::vec3 line2 = Collisions::ClosestPointOnLineSegment(tri1, tri2, point);
	const glm::vec3 line3 = Collisions::ClosestPointOnLineSegment(tri2, tri0, point);
	const float mag1 = LengthSq(point - line1);
	const float mag2 = LengthSq(point - line2);
	const float mag3 = LengthSq(point - line3);

	if (mag1 <= radiussq)
	{
		smallestDist = mag1;
		itx = line1;
	}
	if (mag2 <= radiussq && mag2 < smallestDist)
	{
		smallestDist = mag2;
		itx = line2;
	}
	if (mag3 <= radiussq && mag3 < smallestDist)
	{
		smallestDist = mag3;
		itx = line3;
	}

	if (smallestDist > radiussq) return {};

	norm = point - itx;

	// the sphere is inside the triangle, so the normal is zero instead, just return the triangle's normal
	if (norm.x == 0.0f && norm.y == 0.0f && norm.z == 0.0f) retInfo.normal = triNormal;
	else retInfo.normal = norm;
	retInfo.point = itx;
	retInfo.length = Length(norm);
	return retInfo;
}

inline HitInfo TriangleOnCapsule(const glm::vec3& tri0, const glm::vec3& tri1, const glm::vec3& tri2, const glm::vec3& triNormal, const glm::vec3& tip, const glm::vec3& base, const glm::vec3& a, const glm::vec3& b, const glm::vec3& capn, float radius)
{
	// find the normal of this triangle
	// tbd if necessary, this sometimes fixes weird edgecases
	glm::vec3 side1 = tri1 - tri0;
	glm::vec3 side2 = tri2 - tri0;
	glm::vec3 norm = glm::normalize(glm::cross(side1, side2));

	float dotOfNormals = abs(glm::dot(norm, capn));

	// default reference point to an arbitrary point on the triangle
	// for when dotOfNormals is 0, because then the capsule is parallel to the triangle
	glm::vec3 ref = tri0;
	if (dotOfNormals > 0)
	{
		// capsule is not parallel to the triangle's plane
		// find where the capsule's normal vector intersects the triangle's plane
		float t = glm::dot(norm, (tri0 - base) / dotOfNormals);

		glm::vec3 plane_itx = base + capn * t;
		// then clamp that plane intersect point onto the triangle itself this is the new reference point
		ClosestHitInfo closestInfo = ClosestPointOnTriangle(tri0, tri1, tri2, triNormal, plane_itx);
		ref = closestInfo.point;
	}

	// find the closest point on the capsule line to the reference point
	glm::vec3 c = Collisions::ClosestPointOnLineSegment(a, b, ref);

	// do a sphere cast from that closest point to the triangle and return the result
	return TriangleOnSphere(tri0, tri1, tri2, triNormal, c, radius);
}

inline ClosestHitInfo TriangleCapsuleFindClosest(const Poly& poly,
	const glm::vec3& tip,
	const glm::vec3& base,
	const glm::vec3& a,
	const glm::vec3& b,
	const glm::vec3& norm,
	float radius)
{
	ClosestHitInfo retInfo;

	//declare the variables that will be returned by the function
	float finalLength = HUGE_VALF;
	glm::vec3 w = glm::vec3(0.0f);
	glm::vec3 n = glm::vec3(0.0f);

	// TODO: translation_n � scale_� - �� ������
	float translation_x = 0.0f, translation_y = 0.0f, translation_z = 0.0f,
		scale_x = 1.0f, scale_y = 1.0f, scale_z = 1.0f;

	// ������ ���� �������
	//std::vector<glm::vec3> vertNormal;
	//vertNormal.resize(poly.verts.size());

	for (int i = 0; i < poly.verts.size(); i += 3)
	{
		// apply the function given with the arguments given also supply the points of the current triangle
		// TODO: ������� ������� � ����
		//glm::vec3 normalTriangle = glm::normalize(
		//	verts[v][6] * scale_x,
		//	verts[v][7] * scale_x,
		//	verts[v][8] * scale_x
		//);
		//glm::vec3 normalTriangle = glm::normalize(glm::triangleNormal(
		//	poly.verts[i + 0] * scale_x,
		//	poly.verts[i + 1] * scale_y,
		//	poly.verts[i + 2] * scale_z));
		glm::vec3 normalTriangle = glm::vec3(0.0f);

		auto info = TriangleOnCapsule(
			{ poly.verts[i + 0].x * scale_x + translation_x, poly.verts[i + 0].y * scale_y + translation_y, poly.verts[i + 0].z * scale_z + translation_z },
			{ poly.verts[i + 1].x * scale_x + translation_x, poly.verts[i + 1].y * scale_y + translation_y, poly.verts[i + 1].z * scale_z + translation_z },
			{ poly.verts[i + 2].x * scale_x + translation_x, poly.verts[i + 2].y * scale_y + translation_y, poly.verts[i + 2].z * scale_z + translation_z },
			normalTriangle,
			tip,
			base,
			a,
			b,
			norm,
			radius
		);
		// if something was hit and either the finalLength is not yet defined or the new length is closer then update the collision information
		if (info.length < finalLength)
		{
			finalLength = info.length;
			w = info.point;
			n = info.normal;
		}
	}

	retInfo.length = finalLength;
	retInfo.normal = glm::normalize(n); // normalize the normal vector before it is returned
	retInfo.point = w;
	return retInfo;
}

inline ClosestHitInfo CapsuleIntersection(const Poly& poly,
	const glm::vec3& tip,
	const glm::vec3& base,
	float radius)
{
	// the normal vector coming out the tip of the capsule
	glm::vec3 norm = glm::normalize(tip - base);

	// the baseand tip, inset by the radius
	// these two coordinates are the actual extent of the capsule sphere line
	glm::vec3 a = base + norm * radius;
	glm::vec3 b = tip - norm * radius;

	return TriangleCapsuleFindClosest(poly, tip, base, a, b, norm, radius);
}
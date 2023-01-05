// finds the closest point to the source point on the given line segment
inline constexpr glm::vec3 Collisions::ClosestPointOnLineSegment(
	const glm::vec3& a, // point one of line segment
	const glm::vec3& b, // point two of line segment
	const glm::vec3& point  // source point
)
{
	const glm::vec3 ab = b - a;
	const glm::vec3 pa = point - a;
	const float t = glm::dot(pa, ab) / glm::dot(ab, ab);
	return { a + ab * (t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t) };
}

inline constexpr glm::vec3 Collisions::ClosestPointOnLineSegment(const Line& line, const glm::vec3& point)
{
	return Collisions::ClosestPointOnLineSegment(line.a, line.b, point);
}

inline bool Collisions::CheckPointInTriangle(const glm::vec3& tri0, const glm::vec3& tri1, const glm::vec3& tri2, const glm::vec3& point)
{
	glm::vec3 u = tri1 - tri0;
	glm::vec3 v = tri2 - tri0;
	glm::vec3 w = point - tri0;

	glm::vec3 vw = glm::cross(v, w);
	glm::vec3 vu = glm::cross(v, u);

	if (glm::dot(vw, vu) < 0.0f)
		return false;

	glm::vec3 uw = glm::cross(u, w);
	glm::vec3 uv = glm::cross(u, v);

	if (glm::dot(uw, uv) < 0.0f)
		return false;

	float d = glm::length(uv);
	float r = glm::length(vw) / d;
	float t = glm::length(uw) / d;
	return ((r + t) <= 1.0f);
}

bool Collisions::GetLowestRoot(float a, float b, float c, float maxR, float* root)
{
	// Check if a solution exists
	float determinant = b * b - 4.0f * a * c;

	// If determinant is negative it means no solutions.
	if (determinant < 0.0f) return false;

	// calculate the two roots: (if determinant == 0 then
	// x1==x2 but let’s disregard that slight optimization)
	float sqrtD = sqrt(determinant);
	float r1 = (-b - sqrtD) / (2.0f * a);
	float r2 = (-b + sqrtD) / (2.0f * a);

	// Sort so x1 <= x2
	if (r1 > r2)
	{
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}
	// Get lowest root:
	if (r1 > 0 && r1 < maxR)
	{
		*root = r1;
		return true;
	}
	// It is possible that we want x2 - this can happen
	// if x1 < 0
	if (r2 > 0 && r2 < maxR) 
	{
		*root = r2;
		return true;
	}
	// No (valid) solutions
	return false;
}

// Möller–Trumbore intersection algorithm
inline int Collisions::RayInTri(const glm::vec3& from, const glm::vec3& to, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& intersect)
{
	glm::vec3 vector = glm::normalize(to);
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;

	glm::vec3 h = glm::cross(vector, edge2);
	float a = glm::dot(edge1, h);
	if (a > -FLT_EPSILON && a < FLT_EPSILON)
		return 0;

	float f = 1.0f / a;

	glm::vec3 s = from - v0;

	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return 0;

	glm::vec3 q = glm::cross(s, edge1);

	float v = f * glm::dot(vector, q);
	if (v < 0.0 || u + v > 1.0f)
		return 0;

	float t = f * glm::dot(edge2, q);
	if (t > FLT_EPSILON) 
	{
		glm::vec3 tmp = vector * t;
		intersect = from + tmp;
		return 1;
	}

	return 0;
}

inline glm::vec3 Collisions::TriTriIntersect(const Triangle& t1, const Triangle& t2)
{
	Plane p1(t1.verts[0], t1.verts[1], t1.verts[2]);
	int other_side = 0;
	{
		float f1 = p1.SignedDistanceTo(t2.verts[0]);
		float f2 = p1.SignedDistanceTo(t2.verts[1]);
		float f3 = p1.SignedDistanceTo(t2.verts[2]);
		float f12 = f1 * f2;
		float f23 = f2 * f3;
		if (f12 > 0.0f && f23 > 0.0f) return glm::vec3(0.0f);
		other_side = (f12 < 0.0f ? (f23 < 0.0f ? 1 : 0) : 2);
	}
	Plane p2(t2.verts[0], t2.verts[1], t2.verts[2]);
	glm::vec3 n12(p1.normal + p2.normal);
	TriangleDesc td2(t2, p2);
	const glm::vec3& a2 = td2[other_side + 1];
	const glm::vec3& b2 = td2[other_side];
	const glm::vec3& c2 = td2[other_side + 2];
	float t21 = -(p1.equation.w + p2.equation.w + glm::dot(a2, n12)) / (glm::dot((b2 - a2), n12));
	TriangleDesc td1(t1, p1);
	glm::vec3 P21(a2 + t21 * (b2 - a2));
	if (td1.PointInTri(P21)) return P21;
	float t22 = -(p1.equation.w + p2.equation.w + glm::dot(c2, n12)) / (glm::dot((b2 - c2), n12));
	glm::vec3 P22(c2 + t22 * (b2 - c2));
	if (td1.PointInTri(P22)) return P22;

	{
		float f1 = p2.SignedDistanceTo(t1.verts[0]);
		float f2 = p2.SignedDistanceTo(t1.verts[1]);
		float f3 = p2.SignedDistanceTo(t1.verts[2]);
		float f12 = f1 * f2;
		float f23 = f2 * f3;
		if (f12 > 0.0f && f23 > 0.0f) return glm::vec3(0.0f);
		other_side = (f12 < 0.0f ? (f23 < 0.0f ? 1 : 0) : 2);
	}
	const glm::vec3& a1 = td1[other_side + 1];
	const glm::vec3& b1 = td1[other_side];
	const glm::vec3& c1 = td1[other_side + 2];
	float t11 = -(p1.equation.w + p2.equation.w + glm::dot(a1, n12)) / (glm::dot((b1 - a1), n12));
	glm::vec3 P11(a1 + t11 * (b1 - a1));
	if (td2.PointInTri(P11)) return P11;
	float t12 = -(p1.equation.w + p2.equation.w + glm::dot(c1, n12)) / (glm::dot((b1 - c1), n12));
	glm::vec3 P12(c1 + t12 * (b1 - c1));
	if (td2.PointInTri(P12)) return P12;
	return glm::vec3(0.0f);
}
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
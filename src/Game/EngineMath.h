#pragma once

#include "BaseHeader.h"
#include "Base.h"

//=============================================================================
// Core math
//=============================================================================

static const float Pi = float(3.141592653589793);
static const float HalfPi = float(1.57079632679489661923);

inline constexpr float DegToRad(const float a)
{
	return 0.01745329251994329547f * a;
}

inline float Length(const glm::vec3& v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float LengthSq(const glm::vec3& v)
{
	return (v.x * v.x + v.y * v.y + v.z * v.z);
}

//=============================================================================
// Algebra
//=============================================================================
// ==>Vector/Matrix/Quat/Angle/Axis/Transforms

class Transform
{
public:
	void SetDefault();
	void Update() { updateTransforms(); }

	void Translate(const glm::vec3& position);
	void Translate(float x, float y, float z) { Translate({ x, y, z }); }

	void Move(const glm::vec3& position);
	void Move(float x, float y, float z) { Move({ x, y, z }); }

	void RotateEuler(float x, float y, float z);
	void RotateEuler(const glm::vec3& rotation) { RotateEuler(rotation.x, rotation.y, rotation.z); }

	void Rotate(const glm::quat& rotation);
	void SetRotation(const glm::quat& rotation) { m_rotation = rotation; m_isTransformChanged |= TransformChanged::ROTATION; }

	void Scale(const glm::vec3& scale);
	void Scale(float x, float y, float z) { Scale({ x, y, z }); }

	const glm::vec3& GetPosition() const { return m_position; }
	const glm::vec3& GetWorldPosition() const { return m_worldPosition; }
	const glm::vec3& GetScale() const { return m_scale; }
	const glm::vec3& GetWorldScale() const { return m_worldScale; }
	const glm::quat& GetRotation() const { return m_rotation; }
	const glm::vec3& GetYawPitchRoll() const { return glm::vec3(glm::yaw(m_rotation), glm::pitch(m_rotation), glm::roll(m_rotation)); };
	const glm::quat& GetWorldRotation() const { return m_worldRotation; }

	const glm::vec3& GetForward() const { return m_forward; }
	const glm::vec3& GetUp() const { return m_up; }
	const glm::vec3& GetRight() const { return m_right; }

	const glm::mat4& GetWorld() { updateTransforms(); return m_worldMatrix; }
private:
	void updateTransforms();

	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_worldPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_worldScale = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::quat m_rotation = glm::quat(glm::vec3(0.0f));
	glm::quat m_worldRotation = glm::quat(glm::vec3(0.0f));

	glm::mat4 m_worldMatrix = glm::mat4(1.0f);

	enum TransformChanged {
		NONE = 0x00,
		TRANSLATION = 0x01,
		ROTATION = 0x02,
		SCALE = 0x04,
	};
	uint8_t m_isTransformChanged = TransformChanged::TRANSLATION;

	Transform* m_parent = nullptr; // TODO:
};

//=============================================================================
// Colors
//=============================================================================

inline constexpr unsigned RGBAToUInt(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (unsigned)a << 24 | r << 16 | g << 8 | b;
}

inline constexpr unsigned RGBAFToUInt(float r, float g, float b, float a)
{
	return RGBAToUInt((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255, a * (uint8_t)255);
}

inline constexpr unsigned BGRAToUInt(uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
	return RGBAToUInt(r, g, b, a);
}

inline constexpr unsigned BGRAFToUInt(float b, float g, float r, float a)
{
	return RGBAFToUInt((uint8_t)r * 255, (uint8_t)g * 255, (uint8_t)b * 255, (uint8_t)a * 255);
}

inline constexpr float GetAlphaFromUInt(unsigned rgba)
{
	return (rgba >> 24) / 255.f;
}

inline constexpr glm::vec3 RGBToVec(unsigned rgb)
{
	return { ((rgb >> 16) & 255) / 255.f, ((rgb >> 8) & 255) / 255.f, ((rgb >> 0) & 255) / 255.f };
}

#define RGBX(rgb,x)   ( ((rgb)&0xFFFFFF) | (((unsigned)(x))<<24) )
#define RGB3(r,g,b)   ( ((r)<<16) | ((g)<<8) | (b) )
#define RGB4(r,g,b,a) RGBX(RGB3(r,g,b),a)

#define BLACK   RGBX(0x000000,255)
#define WHITE   RGBX(0xFFF1E8,255)

#if 0
#define RED     RGBX(0xFF004D,255)
#define GREEN   RGBX(0x00B543,255)
#define BLUE    RGBX(0x065AB5,255)
#define ORANGE  RGBX(0xFF6C24,255)
#define CYAN    RGBX(0x29ADFF,255)
#define PURPLE  RGBX(0x7E2553,255)
#define YELLOW  RGBX(0xFFEC27,255)
#define GRAY    RGBX(0x725158,255)
#else
// in this colour scheme, all components make 255+192 (~) to keep
// tone balance. red, purple and yellow tweak a little bit to
// fit better with gray colours.
#define RED     RGB3(   255,48,48 )
#define GREEN   RGB3(  144,255,48 )
#define CYAN    RGB3(   0,192,255 )
#define ORANGE  RGB3(  255,144,48 )
#define PURPLE  RGB3( 178,128,255 )
#define YELLOW  RGB3(   255,224,0 )
#define GRAY    RGB3( 149,149,149 )
#define PINK    RGB3(  255,48,144 )
#define AQUA    RGB3(  48,255,144 )

#define BLUE    RGBX(0x065AB5,255)
#endif

//=============================================================================
// Geometry
//=============================================================================

class Line
{
public:
	Line() = default;
	Line(const Line&) = default;
	Line(const glm::vec3& P0, const glm::vec3& P1)
	{
		a = P0;
		b = P1;
	}

	Line& operator=(const Line&) = default;

	glm::vec3 a = { 0.0f, 0.0f, 0.0f };
	glm::vec3 b = { 0.0f, 0.0f, 0.0f };
};

class Triangle
{
public:
	Triangle() = default;
	Triangle(const Triangle&) = default;
	Triangle(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2)
	{
		verts[0] = P0;
		verts[1] = P1;
		verts[2] = P2;
	}

	Triangle& operator=(const Triangle&) = default;

	// Compute the area of the triangle.
	float Area() const
	{
		const glm::vec3& p0 = verts[0];
		const glm::vec3& p1 = verts[1];
		const glm::vec3& p2 = verts[2];

		return glm::cross((p0 - p1), (p0 - p2)).length() * 0.5f;
	}

	// Computes a point on the triangle from u and v barycentric coordinates.
	glm::vec3 PointFromUV(float u, float v) const
	{
		return (1.0f - u - v) * verts[0] + u * verts[1] + v * verts[2];
	}

	glm::vec3 verts[3] =
	{
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }
	};
};

class Sphere
{
public:
	Sphere() = default;
	Sphere(const Sphere&) = default;
	Sphere(const glm::vec3& Position, float Radius)
	{
		position = Position;
		radius = Radius;
	}

	Sphere& operator=(const Sphere&) = default;

	bool IsValid() const { return radius > 0.0f; }

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	float radius = 1.0f;
};

class Plane
{
public:
	Plane() = default;
	Plane(const Plane&) = default;
	Plane(const glm::vec3& P, const glm::vec3& N)
	{
		p = P;
		normal = N;
	}

	Plane& operator=(const Plane&) = default;

	glm::vec3 p = { 0.0f, 0.0f, 0.0f };
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
};

class AABB
{
public:
	AABB() = default;
	AABB(const AABB&) = default;
	AABB(const glm::vec3& minimum, const glm::vec3& maximum)
	{
		min = minimum;
		max = maximum;
	}

	AABB& operator=(const AABB&) = default;

	static AABB GetCenterExtents(const glm::vec3& center, const glm::vec3& extent)
	{
		return { center - extent, center + extent };
	}

	// TODO: проверить
	static AABB GetBasisExtent(const glm::vec3& center, const glm::mat3& basis, const glm::vec3& extent)
	{
		// extended basis vectors
		const glm::vec3 c0 = basis[0] * extent.x;
		const glm::vec3 c1 = basis[1] * extent.y;
		const glm::vec3 c2 = basis[2] * extent.z;

		// find combination of base vectors that produces max. distance for each component = sum of abs()
		const glm::vec3 w(
			abs(c0.x) + abs(c1.x) + abs(c2.x),
			abs(c0.y) + abs(c1.y) + abs(c2.y),
			abs(c0.z) + abs(c1.z) + abs(c2.z));

		return { center - w, center + w };
	}

	// TODO: проверить
	// TODO: не учитывается скалирование
	static AABB GetPoseExtent(const Transform& pose, const glm::vec3& extent)
	{
		return GetBasisExtent(pose.GetPosition(), glm::toMat3(pose.GetRotation()), extent);
	}

	// gets the transformed bounds of the passed AABB (resulting in a bigger AABB).
	static AABB GetTransform(const glm::mat3& matrix, const AABB& aabb)
	{
		return GetBasisExtent(matrix * aabb.GetCenter(), matrix, aabb.GetExtents());
	}

	// gets the transformed bounds of the passed AABB (resulting in a bigger AABB).
	static AABB GetTransform(const Transform& transform, const AABB& aabb)
	{
		return GetBasisExtent(aabb.GetCenter(), glm::toMat3(transform.GetRotation()), aabb.GetExtents());
	}

	// expands the volume to include v
	void Include(const glm::vec3& v)
	{
		min = Min(min, v);
		max = Max(max, v);
	}

	// expands the volume to include v
	void Include(const AABB& aabb)
	{
		min = Min(min, aabb.min);
		max = Max(max, aabb.max);
	}

	// indicates whether the intersection of this and b is empty or not.
	bool Intersects(const AABB& aabb) const
	{
		return !(aabb.min.x > max.x || min.x > aabb.max.x || aabb.min.y > max.y || min.y > aabb.max.y || aabb.min.z > max.z || min.z > aabb.max.z);
	}

	// computes the 1D-intersection between two AABBs, on a given axis.
	bool Intersects1D(const AABB& aabb, uint32_t axis) const
	{
		return max[axis] >= aabb.min[axis] && aabb.max[axis] >= min[axis];
	}

	// indicates if these bounds contain v.
	bool Contains(const glm::vec3& v) const
	{
		return !(v.x < min.x || v.x > max.x || v.y < min.y || v.y > max.y || v.z < min.z || v.z > max.z);
	}

	// checks a box is inside another box.
	bool IsInside(const AABB& aabb) const
	{
		if (aabb.min.x > min.x)
			return false;
		if (aabb.min.y > min.y)
			return false;
		if (aabb.min.z > min.z)
			return false;
		if (aabb.max.x < max.x)
			return false;
		if (aabb.max.y < max.y)
			return false;
		if (aabb.max.z < max.z)
			return false;
		return true;
	}

	glm::vec3 GetCenter() const
	{
		return (min + max) * 0.5f;
	}

	// get component of the box's center along a given axis
	float GetCenter(uint32_t axis) const
	{
		return (min[axis] + max[axis]) * 0.5f;
	}

	// get component of the box's extents along a given axis
	float GetExtents(uint32_t axis) const
	{
		return (max[axis] - min[axis]) * 0.5f;
	}

	// returns the dimensions (width/height/depth) of this axis aligned box.
	glm::vec3 GetDimensions() const
	{
		return max - min;
	}

	glm::vec3 GetExtents() const
	{
		return GetDimensions() * 0.5f;
	}

	void Scale(float scale)
	{
		*this = GetCenterExtents(GetCenter(), GetExtents() * scale);
	}

	// fattens the AABB in all 3 dimensions by the given distance.
	void Fattens(float distance)
	{
		min.x -= distance;
		min.y -= distance;
		min.z -= distance;

		max.x += distance;
		max.y += distance;
		max.z += distance;
	}

	// Finds the closest point in the box to the point p. If p is contained, this will be p, otherwise it will be the closest point on the surface of the box.
	glm::vec3 ClosestPoint(const glm::vec3& p) const
	{
		return Max(min, Min(max, p));
	}

	glm::vec3 min = { 0.0f, 0.0f, 0.0f };
	glm::vec3 max = { 0.0f, 0.0f, 0.0f };
};

class Capsule
{
public:
	Capsule() = default;
	Capsule(const Capsule&) = default;
	Capsule(const glm::vec3& A, const glm::vec3& B, float R)
	{
		a = A;
		b = B;
		r = R;
	}

	Capsule& operator=(const Capsule&) = default;

	glm::vec3 a = { 0.0f, 0.0f, 0.0f };
	glm::vec3 b = { 0.0f, 0.0f, 0.0f };
	float r = 0.0f;
};

class Ray
{
public:
	Ray() = default;
	Ray(const Ray&) = default;
	Ray(const glm::vec3& P, const glm::vec3& D)
	{
		p = P;
		d = D;
	}

	Ray& operator=(const Ray&) = default;

	glm::vec3 p = { 0.0f, 0.0f, 0.0f };
	glm::vec3 d = { 0.0f, 0.0f, 0.0f };
};

class Poly
{
public:
	std::vector<glm::vec3> verts;
	int cnt = 0; // todo: delete, in verts.size
};




struct Frustum
{
	union
	{
		struct { glm::vec4 l, r, t, b, n, f; }; 
		glm::vec4 pl[6]; 
		float v[24];
	};
};

struct Hit 
{
	union 
	{
		// general case
		float depth;
		// rays only: penetration (t0) and extraction (t1) points along ray line
		struct { float t0, t1; };
		// gjk only
		struct { int hits; glm::vec3 p0, p1; float distance2; int iterations; };
	};
	union
	{ 
		glm::vec3 p; 
		glm::vec3 contact_point; 
	};
	union 
	{ 
		glm::vec3 n; 
		glm::vec3 normal; 
	};
};

namespace tempMath
{
	using namespace glm;

	// потом удалить

#define C_EPSILON  (1e-6)
#define C_PI       (3.141592654f) // (3.14159265358979323846f)
#define TO_RAD     (C_PI/180.f)
#define TO_DEG     (180.f/C_PI)

	inline float deg(float radians) { return radians / C_PI * 180.0f; }
	inline float rad(float degrees) { return degrees * C_PI / 180.0f; }

	inline float minf(float a, float b) { return a < b ? a : b; }
	inline float maxf(float a, float b) { return a > b ? a : b; }
	inline float absf(float a) { return a < 0.0f ? -a : a; }
	inline float pmodf(float a, float b) { return (a < 0.0f ? 1.0f : 0.0f) + (float)fmod(a, b); } // positive mod
	inline float signf(float a) { return (a < 0) ? -1.f : 1.f; }
	inline float clampf(float v, float a, float b) { return maxf(minf(b, v), a); }
	inline float mixf(float a, float b, float t) { return a * (1 - t) + b * t; }

	inline vec2  ptr2(const float* a) { return vec2(a[0], a[1]); }
	//
	inline vec2  neg2(vec2   a) { return vec2(-a.x, -a.y); }
	inline vec2  add2(vec2   a, vec2   b) { return vec2(a.x + b.x, a.y + b.y); }
	inline vec2  sub2(vec2   a, vec2   b) { return vec2(a.x - b.x, a.y - b.y); }
	inline vec2  mul2(vec2   a, vec2   b) { return vec2(a.x * b.x, a.y * b.y); }
	inline vec2  inc2(vec2   a, float  b) { return vec2(a.x + b, a.y + b); }
	inline vec2  dec2(vec2   a, float  b) { return vec2(a.x - b, a.y - b); }
	inline vec2  scale2(vec2   a, float  b) { return vec2(a.x * b, a.y * b); }
	inline vec2  div2(vec2   a, float  b) { return scale2(a, b ? 1 / b : 0.f); }
	inline vec2  pmod2(vec2   a, float  b) { return vec2(pmodf(a.x, b), pmodf(a.y, b)); }
	inline vec2  min2(vec2   a, vec2   b) { return vec2(minf(a.x, b.x), minf(a.y, b.y)); }
	inline vec2  max2(vec2   a, vec2   b) { return vec2(maxf(a.x, b.x), maxf(a.y, b.y)); }
	inline vec2  abs2(vec2   a) { return vec2(absf(a.x), absf(a.y)); }
	inline vec2  floor2(vec2   a) { return vec2(floorf(a.x), floorf(a.y)); }
	inline vec2  fract2(vec2   a) { return sub2(a, floor2(a)); }
	inline vec2  ceil2(vec2   a) { return vec2(ceilf(a.x), ceilf(a.y)); }
	inline float dot2(vec2   a, vec2   b) { return a.x * b.x + a.y * b.y; }
	inline vec2  refl2(vec2   a, vec2   b) { return sub2(a, scale2(b, 2 * dot2(a, b))); }
	inline float cross2(vec2   a, vec2   b) { return a.x * b.y - a.y * b.x; } // pseudo cross product
	inline float len2sq(vec2   a) { return a.x * a.x + a.y * a.y; }
	inline float len2(vec2   a) { return sqrtf(len2sq(a)); }
	inline vec2  norm2(vec2   a) { return /*dot(2) == 0 ? a :*/ div2(a, len2(a)); }
	//inline int   finite2(vec2   a) { return FINITE(a.x) && FINITE(a.y); }
	inline vec2  mix2(vec2 a, vec2 b, float t) { return add2(scale2((a), 1 - (t)), scale2((b), t)); }
	inline vec2  clamp2(vec2 v, float a, float b) { return vec2(maxf(minf(b, v.x), a), maxf(minf(b, v.y), a)); }

	inline vec3 neg3(vec3 a) { return vec3(-a.x, -a.y, -a.z); }
	inline vec3 add3(vec3 a, vec3 b) { return vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
	inline vec3 sub3(vec3 a, vec3 b) { return vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
	inline vec3 mul3(vec3 a, vec3 b) { return vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
	inline vec3 inc3(vec3 a, float b) { return vec3(a.x + b, a.y + b, a.z + b); }
	inline vec3 dec3(vec3 a, float b) { return vec3(a.x - b, a.y - b, a.z - b); }
	inline vec3 scale3(vec3 a, float b) { return vec3(a.x * b, a.y * b, a.z * b); }
	inline vec3 div3(vec3 a, float b) { return scale3(a, b ? 1 / b : 0.f); }

	inline vec3 cross3(vec3 a, vec3 b) { return vec3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y); }
	inline float dot3(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

	inline float len3sq(vec3 a) { return dot3(a, a); }
	inline float len3(vec3 a) { return sqrtf(len3sq(a)); }
	inline vec3 norm3(vec3 a) { return /*dot3(a) == 0 ? a :*/ div3(a, len3(a)); }

	inline vec4  neg4(vec4   a) { return vec4(-a.x, -a.y, -a.z, -a.w); }
	inline vec4  add4(vec4   a, vec4   b) { return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
	inline vec4  sub4(vec4   a, vec4   b) { return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
	inline vec4  mul4(vec4   a, vec4   b) { return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
	inline vec4  inc4(vec4   a, float  b) { return vec4(a.x + b, a.y + b, a.z + b, a.w + b); }
	inline vec4  dec4(vec4   a, float  b) { return vec4(a.x - b, a.y - b, a.z - b, a.w - b); }
	inline vec4  scale4(vec4   a, float  b) { return vec4(a.x * b, a.y * b, a.z * b, a.w * b); }
	inline vec4  div4(vec4   a, float  b) { return scale4(a, b ? 1 / b : 0.f); }

	inline void  ortho3(vec3* left, vec3* up, vec3 v) 
	{
#if 0
		if ((v.z * v.z) > (0.7f * 0.7f)) {
			float sqrlen = v.y * v.y + v.z * v.z;
			float invlen = 1.f / sqrtf(sqrlen);
			*up = vec3(0, v.z * invlen, -v.y * invlen);
			*left = vec3(sqrlen * invlen, -v.x * up->z, v.x * up->y);
		}
		else {
			float sqrlen = v.x * v.x + v.y * v.y;
			float invlen = 1.f / sqrtf(sqrlen);
			*left = vec3(-v.y * invlen, v.x * invlen, 0);
			*up = vec3(-v.z * left->y, v.z * left->x, sqrlen * invlen);
		}
#else
		* left = (v.z * v.z) < (v.x * v.x) ? vec3(v.y, -v.x, 0) : vec3(0, -v.z, v.y);
		*up = cross3(*left, v);
#endif
	}

	inline void rotation33(glm::mat3& M, float degrees, float x, float y, float z) 
	{
		float radians = degrees * C_PI / 180.0f;
		float s = sinf(radians), c = cosf(radians), c1 = 1.0f - c;
		float xy = x * y, yz = y * z, zx = z * x, xs = x * s, ys = y * s, zs = z * s;
		
		float m[9];
		m[0] = c1 * x * x + c; m[1] = c1 * xy - zs; m[2] = c1 * zx + ys;
		m[3] = c1 * xy + zs; m[4] = c1 * y * y + c; m[5] = c1 * yz - xs;
		m[6] = c1 * zx - ys; m[7] = c1 * yz + xs; m[8] = c1 * z * z + c;

		M = glm::make_mat3(m);
	}

	inline void ortho44(glm::mat4& M, float l, float r, float b, float t, float n, float f) 
	{
		float m[16];
		m[0] = 2 / (r - l);      m[1] = 0;            m[2] = 0;            m[3] = 0;
		m[4] = 0;            m[5] = 2 / (t - b);      m[6] = 0;            m[7] = 0;
		m[8] = 0;            m[9] = 0;            m[10] = -2 / (f - n);     m[11] = 0;
		m[12] = -(r + l) / (r - l); m[13] = -(t + b) / (t - b); m[14] = -(f + n) / (f - n); m[15] = 1;

		M = glm::make_mat4(m);
	}
	inline void frustum44(glm::mat4& M, float l, float r, float b, float t, float n, float f)
	{
		float m[16];
		m[0] = 2 * n / (r - l);   m[1] = 0;           m[2] = 0;               m[3] = 0;
		m[4] = 0;           m[5] = 2 * n / (t - b);   m[6] = 0;               m[7] = 0;
		m[8] = (r + l) / (r - l); m[9] = (t + b) / (t - b); m[10] = -(f + n) / (f - n);    m[11] = -1;
		m[12] = 0;           m[13] = 0;           m[14] = -2 * (f * n) / (f - n);  m[15] = 0;

		M = glm::make_mat4(m);
	}
	inline void perspective44(glm::mat4& m, float fovy_degrees, float aspect, float nearp, float farp)
	{
		float y = tanf(fovy_degrees * C_PI / 360) * nearp, x = y * aspect;
		frustum44(m, -x, x, -y, y, nearp, farp);
	}
	inline void lookat44(glm::mat4& M, vec3 eye, vec3 center, vec3 up)
	{
		vec3 f = norm3(sub3(center, eye));
		vec3 r = norm3(cross3(f, up));
		vec3 u = cross3(r, f);
		float m[16];
		m[0] = r.x;           m[1] = u.x;           m[2] = -f.x;         m[3] = 0;
		m[4] = r.y;           m[5] = u.y;           m[6] = -f.y;         m[7] = 0;
		m[8] = r.z;           m[9] = u.z;           m[10] = -f.z;         m[11] = 0;
		m[12] = -dot3(r, eye); m[13] = -dot3(u, eye); m[14] = -dot3(f, eye); m[15] = 1;

		M = glm::make_mat4(m);
	}
}

namespace collide
{
	using namespace tempMath;

	// gjk wrapper

#define GJK_MAX_ITERATIONS 20

	struct GJKSupport
	{
		int aid, bid;
		vec3 a;
		vec3 b;
	};
	
	struct GJKVertex 
	{
		vec3 a;
		vec3 b;
		vec3 p;
		int aid, bid;
	};
	struct GJKSimplex
	{
		int max_iter, iter;
		int hit, cnt;
		GJKVertex v[4];
		float bc[4], D;
	};	

	struct GJKResult 
	{
		int hit;
		vec3 p0;
		vec3 p1;
		float distance_squared;
		int iterations;
	};
	
	int GJK(GJKSimplex* s, const GJKSupport* sup, vec3* dv);
	GJKResult GJKAnalyze(const GJKSimplex* s);
	GJKResult GJKQuad(float a_radius, float b_radius);

	/* line/segment */

	float LineDistance2Point(const Line& l, const glm::vec3& p);
	//glm::vec3 LineClosestPoint(const Line& l, const glm::vec3& p);

	/* ray */

	float RayTestPlane(const Ray& r, const glm::vec4& p4);
	float RayTestTriangle(const Ray& r, const Triangle& t);
	int RayTestSphere(float* t0, float* t1, const Ray& r, const Sphere& s);
	int RayTestAABB(float* t0, float* t1, const Ray& r, const AABB& a);
	Hit* RayHitPlane(const Ray& r, const Plane& p);
	Hit* RayHitTriangle(const Ray& r, const Triangle& t);
	Hit* RayHitSphere(const Ray& r, const Sphere& s);
	Hit* RayHitAABB(const Ray& r, const AABB& a);
	
	/* sphere */

	vec3 SphereClosestPoint(const Sphere& s, vec3 p);
	Hit* SphereHitAABB(const Sphere& s, const AABB& a);
	Hit* SphereHitCapsule(Sphere s, const Capsule& c);
	Hit* SphereHitSphere(const Sphere& a, const Sphere& b);
	int SphereTestAABB(const Sphere& s, const AABB& a);
	int SphereTestCapsule(const Sphere& s, const Capsule& c);
	int SphereTestPoly(const Sphere& s, const Poly& p);
	int SphereTestSphere(const Sphere& a, const Sphere& b);

	/* aabb */

	vec3 AABBClosestPoint(const AABB& a, vec3 p);
	float AABBDistance2Point(const AABB& a, vec3 p);
	int  AABBContainsPoint(const AABB& a, vec3 p);
	Hit* AABBHitAABB(const AABB& a, const AABB& b);
	Hit* AABBHitCapsule(const AABB& a, const Capsule& c);
	Hit* AABBHitSphere(const AABB& a, const Sphere& s);
	int AABBTestAABB(const AABB& a, const AABB& b);
	int AABBTestCapsule(const AABB& a, const Capsule& c);
	int AABBTestPoly(const AABB& a, const Poly& p);
	int AABBTestSphere(const AABB& a, const Sphere& s);

	/* capsule */

	float CapsuleDistance2Point(const Capsule& c, vec3 p);
	vec3 CapsuleClosestPoint(const Capsule& c, vec3 p);
	Hit* CapsuleHitAABB(const Capsule& c, const AABB& a);
	Hit* CapsuleHitCapsule(const Capsule& a, const Capsule& b);
	Hit* CapsuleHitSphere(const Capsule& c, const Sphere& s);
	int CapsuleTestAABB(const Capsule& c, const AABB& a);
	int CapsuleTestCapsule(const Capsule& a, const Capsule& b);
	int CapsuleTestPoly(const Capsule& c, const Poly& p);
	int CapsuleTestSphere(const Capsule& c, const Sphere& s);

	/* poly: query */

	int PolyTestSphere(const Poly& p, const Sphere& s);
	int PolyTestAABB(const Poly& p, const AABB& a);
	int PolyTestCapsule(const Poly& p, const Capsule& c);
	int PolyTestPoly(const Poly& a, const Poly& b);

	/* poly: query transformed */

	int PolyTestSphereTransform(const Poly& p, vec3 pos3, const glm::mat3& rot33, const Sphere& s);
	int PolyTestAABBTransform(const Poly& p, vec3 apos3, const glm::mat3& arot33, const AABB& a);
	int PolyTestCapsuleTransform(const Poly& p, vec3 pos3, const glm::mat3& rot33, const Capsule& c);
	int PolyTestPolyTransform(const Poly& a, vec3 apos3, const glm::mat3& arot33, const Poly& b, vec3 bpos3, const glm::mat3& brot33);

	/* poly: gjk result */

	int PolyHitSphere(GJKResult* res, const Poly& p, const Sphere& s);
	int PolyHitAABB(GJKResult* res, const Poly& p, const AABB& a);
	int PolyHitCapsule(GJKResult* res, const Poly& p, const Capsule& c);
	int PolyHitPoly(GJKResult* res, const Poly& a, const Poly& b);

	/* poly: gjk result transformed */

	int PolyHitSphereTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const Sphere& s);
	int PolyHitAABBTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const AABB& a);
	int PolyHitCapsuleTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const Capsule& c);
	int PolyHitPolyTransform(GJKResult* res, const Poly& a, vec3 at3, const glm::mat3& ar33, const Poly& b, vec3 bt3, const glm::mat3& br33);

	vec4 Plane4(vec3 p, vec3 n);

	Frustum FrustumBuild(const glm::mat4& projview);
	int     FrustumTestSphere(const Frustum& f, const Sphere& s);
	int     FrustumTestAABB(const Frustum& f, const AABB& a);

	Poly Pyramid(const glm::vec3& from, const glm::vec3& to, float size);
	Poly Diamond(const glm::vec3& from, const glm::vec3& to, float size);
}

#include "EngineMath.inl"


//=============================================================================
// OLD CODE
//=============================================================================

enum class VolumeCheck
{
	OUTSIDE,
	INTERSECT,
	CONTAINS
};

struct Plane2
{
	Plane2() = default;
	Plane2(const glm::vec3& normal, const glm::vec3& det)
	{
		n = normal;
		d = det;
	}
	Plane2(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		d = a;
		n = glm::normalize(glm::cross(-b + a, c - a));
	}

	glm::vec3 n = glm::vec3(0, 1, 0);
	glm::vec3 d = glm::vec3(0, 0, 0);
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

class OldFrustum
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
	std::vector<Plane2> m_planes;
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
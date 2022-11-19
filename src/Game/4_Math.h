#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

//#if defined(_MSC_VER) && (_MSC_VER <= 1700)
//#define FINITE _finite
//#else
//#define FINITE isfinite
//#endif


// colors

inline unsigned rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (unsigned)a << 24 | r << 16 | g << 8 | b;
}

inline unsigned bgra(uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
	return rgba(r, g, b, a);
}

inline unsigned rgbaf(float r, float g, float b, float a)
{
	return rgba(r * 255, g * 255, b * 255, a * 255);
}

inline unsigned bgraf(float b, float g, float r, float a)
{
	return rgba(r * 255, g * 255, b * 255, a * 255);
}

inline float    alpha(unsigned rgba)
{
	return (rgba >> 24) / 255.f;
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

inline glm::vec3 rgbf(unsigned rgb)
{
	return { ((rgb >> 16) & 255) / 255.f,((rgb >> 8) & 255) / 255.f,((rgb >> 0) & 255) / 255.f };
}

struct Line
{
	glm::vec3 a = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 b = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Sphere
{
	Sphere() = default;
	Sphere(const glm::vec3& position, float size)
	{
		pos = position;
		radius = size;
	}
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
	float radius = 1.0f;
};

struct AABB
{
	glm::vec3 min = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 max = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Plane 
{ 
	glm::vec3 p = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 n = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Capsule
{
	glm::vec3 a = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 b = glm::vec3(0.0f, 0.0f, 0.0f);
	float r = 0.0f;
};


struct Ray
{
	glm::vec3 p = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 d = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Triangle
{
	glm::vec3 p0 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 p1 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 p2 = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct Poly
{
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
	/*inline gjk_support ToGJKSupport(const GJKSupport& gjk)
	{
		gjk_support ret = { 0 };
		ret.aid = gjk.aid;
		ret.bid = gjk.bid;

		ret.a[0] = gjk.a.x;
		ret.a[1] = gjk.a.y;
		ret.a[2] = gjk.a.z;

		ret.b[0] = gjk.b.x;
		ret.b[1] = gjk.b.y;
		ret.b[2] = gjk.b.z;

		return ret;
	}
	inline GJKSupport ToGJKSupport(const gjk_support& gjk)
	{
		GJKSupport ret = { 0 };
		ret.aid = gjk.aid;
		ret.bid = gjk.bid;

		ret.a.x = gjk.a[0];
		ret.a.y = gjk.a[1];
		ret.a.z = gjk.a[2];

		ret.b.x = gjk.b[0];
		ret.b.y = gjk.b[1];
		ret.b.z = gjk.b[2];

		return ret;
	}*/

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
	//inline gjk_simplex ToGJKSimplex(const GJKSimplex& gjk)
	//{
	//	gjk_simplex ret;

	//	ret.max_iter = gjk.max_iter;
	//	ret.iter = gjk.iter;
	//	ret.hit = gjk.hit;
	//	ret.cnt = gjk.cnt;

	//	for (int i = 0; i < 4; i++)
	//	{
	//		ret.v[i].a[0] = gjk.v[i].a.x;
	//		ret.v[i].a[1] = gjk.v[i].a.y;
	//		ret.v[i].a[2] = gjk.v[i].a.z;

	//		ret.v[i].b[0] = gjk.v[i].b.x;
	//		ret.v[i].b[1] = gjk.v[i].b.y;
	//		ret.v[i].b[2] = gjk.v[i].b.z;

	//		ret.v[i].p[0] = gjk.v[i].p.x;
	//		ret.v[i].p[1] = gjk.v[i].p.y;
	//		ret.v[i].p[2] = gjk.v[i].p.z;

	//		ret.v[i].aid = gjk.v[i].aid;
	//		ret.v[i].bid = gjk.v[i].bid;

	//		ret.bc[i] = gjk.bc[i];
	//	}

	//	ret.D = gjk.D;

	//	return ret;
	//}
	//inline GJKSimplex ToGJKSimplex(const gjk_simplex& gjk)
	//{
	//	GJKSimplex ret;

	//	ret.max_iter = gjk.max_iter;
	//	ret.iter = gjk.iter;
	//	ret.hit = gjk.hit;
	//	ret.cnt = gjk.cnt;

	//	for (int i = 0; i < 4; i++)
	//	{
	//		ret.v[i].a.x = gjk.v[i].a[0];
	//		ret.v[i].a.y = gjk.v[i].a[1];
	//		ret.v[i].a.z = gjk.v[i].a[2];

	//		ret.v[i].b.x = gjk.v[i].b[0];
	//		ret.v[i].b.y = gjk.v[i].b[1];
	//		ret.v[i].b.z = gjk.v[i].b[2];

	//		ret.v[i].p.x = gjk.v[i].p[0];
	//		ret.v[i].p.y = gjk.v[i].p[1];
	//		ret.v[i].p.z = gjk.v[i].p[2];

	//		ret.v[i].aid = gjk.v[i].aid;
	//		ret.v[i].bid = gjk.v[i].bid;

	//		ret.bc[i] = gjk.bc[i];
	//	}

	//	ret.D = gjk.D;

	//	return ret;
	//}

	struct GJKResult 
	{
		int hit;
		vec3 p0;
		vec3 p1;
		float distance_squared;
		int iterations;
	};
	//inline gjk_result ToGJKResult(const GJKResult& gjk)
	//{
	//	gjk_result ret;
	//	ret.hit = gjk.hit;

	//	ret.p0[0] = gjk.p0.x;
	//	ret.p0[1] = gjk.p0.y;
	//	ret.p0[2] = gjk.p0.z;

	//	ret.p1[0] = gjk.p1.x;
	//	ret.p1[1] = gjk.p1.y;
	//	ret.p1[2] = gjk.p1.z;

	//	ret.distance_squared = gjk.distance_squared;
	//	ret.iterations = gjk.iterations;

	//	return ret;
	//}
	//inline GJKResult ToGJKResult(const gjk_result& gjk)
	//{
	//	GJKResult ret;
	//	ret.hit = gjk.hit;

	//	ret.p0.x = gjk.p0[0];
	//	ret.p0.y = gjk.p0[1];
	//	ret.p0.z = gjk.p0[2];

	//	ret.p1.x = gjk.p1[0];
	//	ret.p1.y = gjk.p1[1];
	//	ret.p1.z = gjk.p1[2];

	//	ret.distance_squared = gjk.distance_squared;
	//	ret.iterations = gjk.iterations;

	//	return ret;
	//}

	int GJK(GJKSimplex* s, const GJKSupport* sup, vec3* dv);
	GJKResult GJKAnalyze(const GJKSimplex* s);
	GJKResult GJKQuad(float a_radius, float b_radius);

	/* line/segment */

	float LineDistance2Point(const Line& l, const glm::vec3& p);
	glm::vec3 LineClosestPoint(const Line& l, const glm::vec3& p);

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

#include "4_Math.inl"


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
				//return vectorMagnitude(n), itx_x, itx_y, itx_z, tri_n_x, tri_n_y, tri_n_z
			}

			//return vectorMagnitude(n), itx_x, itx_y, itx_z, n_x, n_y, n_z
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
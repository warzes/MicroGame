#include "Collisions.h"

namespace collide
{
	static inline Hit hits[16] = { 0 };
	static inline int hit_index = -1;
#define HitNext() &hits[ (++hit_index) & 15 ]

	inline float lineClosestLine_(float* t1, float* t2, vec3* c1, vec3* c2, Line l, Line m) {
		vec3 r, d1, d2;
		d1 = sub3(l.b, l.a); /* direction vector segment s1 */
		d2 = sub3(m.b, m.a); /* direction vector segment s2 */
		r = sub3(l.a, m.a);

		float i = dot3(d1, d1);
		float e = dot3(d2, d2);
		float f = dot3(d2, r);

		if (i <= C_EPSILON && e <= C_EPSILON) {
			/* both segments degenerate into points */
			vec3 d12;
			*t1 = *t2 = 0.0f;
			*c1 = l.a;
			*c2 = m.a;
			d12 = sub3(*c1, *c2);
			return dot3(d12, d12);
		}
		if (i > C_EPSILON) {
			float c = dot3(d1, r);
			if (e > C_EPSILON) {
				/* non-degenerate case */
				float b = dot3(d1, d2);
				float denom = i * e - b * b;

				/* compute closest point on L1/L2 if not parallel else pick any t2 */
				if (denom != 0.0f)
					*t1 = clampf(0.0f, (b * f - c * e) / denom, 1.0f);
				else *t1 = 0.0f;

				/* cmpute point on L2 closest to S1(s) */
				*t2 = (b * (*t1) + f) / e;
				if (*t2 < 0.0f) {
					*t2 = 0.0f;
					*t1 = clampf(0.0f, -c / i, 1.0f);
				}
				else if (*t2 > 1.0f) {
					*t2 = 1.0f;
					*t1 = clampf(0.0f, (b - c) / i, 1.0f);
				}
			}
			else {
				/* second segment degenerates into a point */
				*t1 = clampf(0.0f, -c / i, 1.0f);
				*t2 = 0.0f;
			}
		}
		else {
			/* first segment degenerates into a point */
			*t2 = clampf(0.0f, f / e, 1.0f);
			*t1 = 0.0f;
		}
		/* calculate closest points */
		vec3 n, d12;
		n = scale3(d1, *t1);
		*c1 = add3(l.a, n);
		n = scale3(d2, *t2);
		*c2 = add3(m.a, n);

		/* calculate squared distance */
		d12 = sub3(*c1, *c2);
		return dot3(d12, d12);
	}

	inline int PolySupport(vec3* support, const vec3& d, const Poly& p) 
	{
		int imax = 0;
		float dmax = dot3(p.verts[0], d);
		for (int i = 1; i < p.cnt; ++i)
		{
			/* find vertex with max dot product in direction d */
			float dot = dot3(p.verts[i], d);
			if (dot < dmax) continue;
			imax = i, dmax = dot;
		} *support = p.verts[imax];
		return imax;
	}

	inline int LineSupport(vec3* support, vec3 d, vec3 a, vec3 b)
	{
		int i = 0;
		float adot = dot3(a, d);
		float bdot = dot3(b, d);
		if (adot < bdot) 
		{
			*support = b;
			i = 1;
		}
		else *support = a;
		return i;
	}

	inline void transform_(vec3* r, vec3 v, const float* r33, vec3 t3) 
	{
		for (int i = 0; i < 3; ++i) {
			i[&r->x] = i[&v.x] * r33[i * 3 + 0];
			i[&r->x] += i[&v.x] * r33[i * 3 + 1];
			i[&r->x] += i[&v.x] * r33[i * 3 + 2];
			i[&r->x] += i[&t3.x];
		}
	}
	inline void transformS(vec3* v, const float* r33, vec3 t3)
	{
		vec3 tmp = *v;
		transform_(v, tmp, r33, t3);
	}
	inline void transformT(vec3* r, vec3 v, const float* r33, vec3 t3) 
	{
		for (int i = 0; i < 3; ++i)
		{
			float p = i[&v.x] - i[&t3.x];
			i[&r->x] = p * r33[0 * 3 + i];
			i[&r->x] += p * r33[1 * 3 + i];
			i[&r->x] += p * r33[2 * 3 + i];
		}
	}
	inline void transformST(vec3* v, const float* r33, vec3 t3)
	{
		vec3 tmp = *v;
		transformT(v, tmp, r33, t3);
	}

	inline float LineDistance2Point(const Line& l, const glm::vec3& p)
	{
		vec3 ab = sub3(l.a, l.b), ap = sub3(l.a, p), bp = sub3(l.b, p);
		/* handle cases p proj outside ab */
		float e = dot3(ap, ab); if (e <= 0) return dot3(ap, ap);
		float f = dot3(ab, ab); if (e >= f) return dot3(bp, bp);
		return dot3(ap, ap) - (e * e) / f;
	}

	inline glm::vec3 LineClosestPoint(const Line& l, const glm::vec3& p)
	{
		vec3 ab = sub3(l.b, l.a), pa = sub3(p, l.a);
		float t = dot3(pa, ab) / dot3(ab, ab);
		return add3(l.a, scale3(ab, t < 0 ? 0 : t > 1 ? 1 : t));
	}

	inline float RayTestPlane(const Ray& r, const glm::vec4& plane)
	{
        /* Ray: P = origin + rd * t
		 * Plane: plane_normal * P + d = 0
		 *
		 * Substitute:
		 *      normal * (origin + rd*t) + d = 0
		 *
		 * Solve for t:
		 *      plane_normal * origin + plane_normal * rd*t + d = 0
		 *      -(plane_normal*rd*t) = plane_normal * origin + d
		 *
		 *                  plane_normal * origin + d
		 *      t = -1 * -------------------------
		 *                  plane_normal * rd
		 *
		 * Result:
		 *      Behind: t < 0
		 *      Infront: t >= 0
		 *      Parallel: t = 0
		 *      Intersection point: ro + rd * t
		 */
		vec3 p = { plane.x, plane.y, plane.z };
        float n = -(dot3(p, r.p) + plane.w);
        if (fabs(n) < 0.0001f) return 0.0f;
        return n / (dot3(p, r.d));
	}

	inline float RayTestTriangle(const Ray& r, const Triangle& tr)
	{
		float t = 0;
		vec3 di0, di1, di2;
		vec3 d21, d02, in;
		vec3 n, d10, d20;
		vec3 in0, in1, in2;

		/* calculate triangle normal */
		d10 = sub3(tr.verts[1], tr.verts[0]);
		d20 = sub3(tr.verts[2], tr.verts[0]);
		d21 = sub3(tr.verts[2], tr.verts[1]);
		d02 = sub3(tr.verts[0], tr.verts[2]);
		n = cross3(d10, d20);

		/* check for plane intersection */
		vec4 p = Plane4(tr.verts[0], n);
		t = RayTestPlane(r, p);
		if (t <= 0.0f) return t;

		/* intersection point */
		in = scale3(r.d, t);
		in = add3(in, r.p);

		/* check if point inside triangle in plane */
		di0 = sub3(in, tr.verts[0]);
		di1 = sub3(in, tr.verts[1]);
		di2 = sub3(in, tr.verts[2]);

		in0 = cross3(d10, di0);
		in1 = cross3(d21, di1);
		in2 = cross3(d02, di2);

		if (dot3(in0, n) < 0.0f)
			return -1;
		if (dot3(in1, n) < 0.0f)
			return -1;
		if (dot3(in2, n) < 0.0f)
			return -1;
		return t;
	}

	inline int RayTestSphere(float* t0, float* t1, const Ray& r, const Sphere& s)
	{
		vec3 a;
		float tc, td, d2, r2;
		a = sub3(s.position, r.p);
		tc = dot3(r.d, a);
		if (tc < 0) return 0;

		r2 = s.radius * s.radius;
		d2 = dot3(a, a) - tc * tc;
		if (d2 > r2) return 0;
		td = sqrtf(r2 - d2);

		*t0 = tc - td;
		*t1 = tc + td;
		return 1;
	}

	inline int RayTestAABB(float* t0, float* t1, const Ray& r, const AABB& a)
	{
		float t0x = (a.min.x - r.p.x) / r.d.x;
		float t0y = (a.min.y - r.p.y) / r.d.y;
		float t0z = (a.min.z - r.p.z) / r.d.z;
		float t1x = (a.max.x - r.p.x) / r.d.x;
		float t1y = (a.max.y - r.p.y) / r.d.y;
		float t1z = (a.max.z - r.p.z) / r.d.z;

		float tminx = minf(t0x, t1x);
		float tminy = minf(t0y, t1y);
		float tminz = minf(t0z, t1z);
		float tmaxx = maxf(t0x, t1x);
		float tmaxy = maxf(t0y, t1y);
		float tmaxz = maxf(t0z, t1z);
		if (tminx > tmaxy || tminy > tmaxx)
			return 0;

		*t0 = maxf(tminx, tminy);
		*t1 = minf(tmaxy, tmaxx);
		if (*t0 > tmaxz || tminz > *t1)
			return 0;

		*t0 = maxf(*t0, tminz);
		*t1 = minf(*t1, tmaxz);
		return 1;
	}

	inline Hit* RayHitPlane(const Ray& r, const Plane& p)
	{
		vec4 pf = Plane4(p.p, p.normal);
		float t = RayTestPlane(r, pf);
		if (t <= 0.0f) return 0;
		Hit* o = HitNext();
		o->p = add3(r.p, scale3(r.d, t));
		o->t0 = o->t1 = t;
		o->n = scale3(p.normal, -1.0f);
		return o;
	}

	inline Hit* RayHitTriangle(const Ray& r, const Triangle& tr)
	{
		float t = RayTestTriangle(r, tr);
		if (t <= 0) return 0;

		Hit* o = HitNext();
		o->t0 = o->t1 = t;
		o->p = add3(r.p, scale3(r.d, t));
		o->n = norm3(cross3(sub3(tr.verts[1], tr.verts[0]), sub3(tr.verts[2], tr.verts[0])));
		return o;
	}

	inline Hit* RayHitSphere(const Ray& r, const Sphere& s)
	{
		Hit* o = HitNext();
		if (!RayTestSphere(&o->t0, &o->t1, r, s))
			return 0;
		o->p = add3(r.p, scale3(r.d, minf(o->t0, o->t1)));
		o->n = norm3(sub3(o->p, s.position));
		return o;
	}

	inline Hit* RayHitAABB(const Ray& r, const AABB& a)
	{
		Hit* o = HitNext();

		vec3 pnt, ext, c;
		float d, min;
		if (!RayTestAABB(&o->t0, &o->t1, r, a))
			return 0;

		o->p = add3(r.p, scale3(r.d, minf(o->t0, o->t1)));
		ext = sub3(a.max, a.min);
		c = add3(a.min, scale3(ext, 0.5f));
		pnt = sub3(o->p, c);

		min = fabs(ext.x - fabs(pnt.x));
		o->n = scale3(vec3(1, 0, 0), signf(pnt.x));
		d = fabs(ext.y - fabs(pnt.y));
		if (d < min) {
			min = d;
			o->n = scale3(vec3(0, 1, 0), signf(pnt.y));
		}
		d = fabs(ext.z - fabs(pnt.z));
		if (d < min)
			o->n = scale3(vec3(0, 0, 1), signf(pnt.z));
		return o;
	}

	inline glm::vec3 SphereClosestPoint(const Sphere& s, vec3 p)
	{
		vec3 d = norm3(sub3(p, s.position));
		return add3(s.position, scale3(d, s.radius));
	}

	inline Hit* SphereHitAABB(const Sphere& s, const AABB& a)
	{
		/* find closest aabb point to sphere center point */
		vec3 ap = AABBClosestPoint(a, s.position);
		vec3 d = sub3(s.position, ap);
		float d2 = dot3(d, d);
		if (d2 > s.radius * s.radius) return 0;

		Hit* m = HitNext();
		/* calculate distance vector between sphere and aabb center points */
		vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
		d = sub3(ac, s.position);

		/* normalize distance vector */
		float l2 = dot3(d, d);
		float l = l2 != 0.0f ? sqrtf(l2) : 1.0f;
		float linv = 1.0f / l;
		d = scale3(d, linv);

		m->normal = d;
		m->contact_point = scale3(m->normal, s.radius);
		m->contact_point = add3(s.position, m->contact_point);

		/* calculate penetration depth */
		vec3 sp = SphereClosestPoint(s, ap);
		d = sub3(sp, ap);
		m->depth = sqrtf(dot3(d, d)) - l;
		return m;
	}

	inline Hit* SphereHitCapsule(Sphere s, const Capsule& c)
	{
#if 0
		// original code
		/* find closest capsule point to sphere center point */
		Hit* m = HitNext();
		vec3 cp = capsule_closest_point(c, s.pos);
		m->normal = sub3(cp, s.pos);
		float d2 = dot3(m->normal, m->normal);
		if (d2 > s.radius * s.radius) return 0;

		/* normalize hit normal vector */
		m->normal = norm3(m->normal);

		/* calculate penetration depth */
		m->depth = d2 - s.radius * s.radius;
		m->depth = m->depth != 0.0f ? sqrtf(m->depth) : 0.0f;
		m->contact_point = add3(s.position, scale3(m->normal, s.radius));
		return m;
#else
		// aproximation of I would expect this function to return instead
		vec3 l = sub3(c.a, c.b); float len = len3(l);
		vec3 d = norm3(l);
		Ray r = Ray(add3(c.a, scale3(d, -2 * len)), d);
		s.radius += c.r;
		Hit* h = RayHitSphere(r, s);
		if (!h) return 0;
		s.radius -= c.r;
		h->contact_point = add3(s.position, scale3(norm3(sub3(h->contact_point, s.position)), s.radius));
		return h;
#endif
	}

	inline Hit* SphereHitSphere(const Sphere& a, const Sphere& b)
	{
		vec3 d = sub3(b.position, a.position);
		float r = a.radius + b.radius;
		float d2 = dot3(d, d);
		if (d2 > r * r) return 0;

		Hit* m = HitNext();
		float l = sqrtf(d2);
		float linv = 1.0f / ((l != 0) ? l : 1.0f);
		m->normal = scale3(d, linv);
		m->depth = r - l;
		d = scale3(m->normal, b.radius);
		m->contact_point = sub3(b.position, d);
		return m;
	}

	inline int SphereTestAABB(const Sphere& s, const AABB& a)
	{
		return AABBTestSphere(a, s);
	}

	inline int SphereTestCapsule(const Sphere& s, const Capsule& c)
	{
		return CapsuleTestSphere(c, s);
	}

	inline int SphereTestPoly(const Sphere& s, const Poly& p)
	{
		return PolyTestSphere(p, s);
	}

	inline int SphereTestSphere(const Sphere& a, const Sphere& b)
	{
		vec3 d = sub3(b.position, a.position);
		float r = a.radius + b.radius;
		if (dot3(d, d) > r * r)
			return 0;
		return 1;
	}

	inline vec3 AABBClosestPoint(const AABB& a, vec3 p)
	{
		vec3 res;
		for (int i = 0; i < 3; ++i)
		{
			float v = i[&p.x];
			if (v < i[&a.min.x]) v = i[&a.min.x];
			if (v > i[&a.max.x]) v = i[&a.max.x];
			i[&res.x] = v;
		}
		return res;
	}

	inline float AABBDistance2Point(const AABB& a, vec3 p)
	{
		float r = 0;
		for (int i = 0; i < 3; ++i) 
		{
			float v = i[&p.x];
			if (v < i[&a.min.x]) r += (i[&a.min.x] - v) * (i[&a.min.x] - v);
			if (v > i[&a.max.x]) r += (v - i[&a.max.x]) * (v - i[&a.max.x]);
		} 
		return r;
	}

	inline int  AABBContainsPoint(const AABB& a, vec3 p)
	{
		if (p.x < a.min.x || p.x > a.max.x) return 0;
		if (p.y < a.min.y || p.y > a.max.y) return 0;
		if (p.z < a.min.z || p.z > a.max.z) return 0;
		return 1;
	}

	inline Hit* AABBHitAABB(const AABB& a, const AABB& b)
	{
		if (!AABBTestAABB(a, b))
			return 0;

		Hit* m = HitNext();
		/* calculate distance vector between both aabb center points */
		vec3 ac, bc, d;
		ac = sub3(a.max, a.min);
		bc = sub3(b.max, b.min);

		ac = scale3(ac, 0.5f);
		bc = scale3(bc, 0.5f);

		ac = add3(a.min, ac);
		bc = add3(b.min, bc);
		d = sub3(bc, ac);

		/* normalize distance vector */
		float l2 = dot3(d, d);
		float l = l2 != 0.0f ? sqrtf(l2) : 1.0f;
		float linv = 1.0f / l;
		d = scale3(d, linv);

		/* calculate contact point */
		m->normal = d;
		m->contact_point = AABBClosestPoint(a, bc);
		d = sub3(m->contact_point, ac);

		/* calculate penetration depth */
		float r2 = dot3(d, d);
		float r = sqrtf(r2);
		m->depth = r - l;
		return m;
	}

	inline Hit* AABBHitCapsule(const AABB& a, const Capsule& c)
	{
		/* calculate aabb center point */
		vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

		/* calculate closest point from aabb to point on capsule and check if inside aabb */
		vec3 cp = CapsuleClosestPoint(c, ac);
		if (!AABBContainsPoint(a, cp))
			return 0;

		Hit* m = HitNext();
		/* vector and distance between both capsule closests point and aabb center*/
		vec3 d; float d2;
		d = sub3(cp, ac);
		d2 = dot3(d, d);

		/* calculate penetration depth from closest aabb point to capsule */
		vec3 ap = AABBClosestPoint(a, cp);
		vec3 dt = sub3(ap, cp);
		m->depth = sqrtf(dot3(dt, dt));

		/* calculate normal */
		float l = sqrtf(d2);
		float linv = 1.0f / ((l != 0.0f) ? l : 1.0f);
		m->normal = scale3(d, linv);
		m->contact_point = ap;
		return m;
	}

	inline Hit* AABBHitSphere(const AABB& a, const Sphere& s)
	{
		/* find closest aabb point to sphere center point */
		Hit* m = HitNext();
		m->contact_point = AABBClosestPoint(a, s.position);
		vec3 d = sub3(s.position, m->contact_point);
		float d2 = dot3(d, d);
		if (d2 > s.radius * s.radius) return 0;

		/* calculate distance vector between aabb and sphere center points */
		vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));
		d = sub3(s.position, ac);

		/* normalize distance vector */
		float l2 = dot3(d, d);
		float l = l2 != 0.0f ? sqrtf(l2) : 1.0f;
		float linv = 1.0f / l;
		d = scale3(d, linv);

		/* calculate penetration depth */
		m->normal = d;
		d = sub3(m->contact_point, ac);
		m->depth = sqrtf(dot3(d, d));
		return m;
	}

	inline int AABBTestAABB(const AABB& a, const AABB& b)
	{
		if (a.max.x < b.min.x || a.min.x > b.max.x) return 0;
		if (a.max.y < b.min.y || a.min.y > b.max.y) return 0;
		if (a.max.z < b.min.z || a.min.z > b.max.z) return 0;
		return 1;
	}

	inline int AABBTestCapsule(const AABB& a, const Capsule& c)
	{
		return CapsuleTestAABB(c, a);
	}

	inline int AABBTestPoly(const AABB& a, const Poly& p)
	{
		return PolyTestAABB(p, a);
	}

	inline int AABBTestSphere(const AABB& a, const Sphere& s)
	{
		/* compute squared distance between sphere center and aabb */
		float d2 = AABBDistance2Point(a, s.position);
		/* intersection if distance is smaller/equal sphere radius*/
		return d2 <= s.radius * s.radius;
	}

	inline float CapsuleDistance2Point(const Capsule& c, vec3 p)
	{
		float d2 = LineDistance2Point(Line(c.a, c.b), p);
		return d2 - (c.r * c.r);
	}

	inline vec3 CapsuleClosestPoint(const Capsule& c, vec3 p)
	{
		/* calculate closest point to internal capsule segment */
		vec3 pp = LineClosestPoint(Line(c.a, c.b), p);

		/* extend point out by radius in normal direction */
		vec3 d = norm3(sub3(p, pp));
		return add3(pp, scale3(d, c.r));
	}

	inline Hit* CapsuleHitAABB(const Capsule& c, const AABB& a)
	{
		/* calculate aabb center point */
		vec3 ac = add3(a.min, scale3(sub3(a.max, a.min), 0.5f));

		/* calculate closest point from aabb to point on capsule and check if inside aabb */
		vec3 cp = CapsuleClosestPoint(c, ac);
		if (!AABBContainsPoint(a, cp))
			return 0;

		Hit* m = HitNext();
		/* vector and distance between both capsule closests point and aabb center*/
		vec3 d; float d2;
		d = sub3(ac, cp);
		d2 = dot3(d, d);

		/* calculate penetration depth from closest aabb point to capsule */
		vec3 ap = AABBClosestPoint(a, cp);
		vec3 dt = sub3(ap, cp);
		m->depth = sqrtf(dot3(dt, dt));

		/* calculate normal */
		float l = sqrtf(d2);
		float linv = 1.0f / ((l != 0.0f) ? l : 1.0f);
		m->normal = scale3(d, linv);
		m->contact_point = cp;
		return m;
	}

	inline Hit* CapsuleHitCapsule(const Capsule& a, const Capsule& b)
	{
		float t1, t2;
		vec3 c1, c2;
		float d2 = lineClosestLine_(&t1, &t2, &c1, &c2, Line(a.a, a.b), Line(b.a, b.b));
		float r = a.r + b.r;
		if (d2 > r * r) return 0;

		Hit* m = HitNext();
		/* calculate normal from both closest points for each segement */
		vec3 cp, d;
		m->normal = sub3(c2, c1);
		m->normal = norm3(m->normal);

		/* calculate contact point from closest point and depth */
		m->contact_point = CapsuleClosestPoint(a, c2);
		cp = CapsuleClosestPoint(b, c1);
		d = sub3(c1, cp);
		m->depth = sqrtf(dot3(d, d));
		return m;
	}

	inline Hit* CapsuleHitSphere(const Capsule& c, const Sphere& s)
	{
		/* find closest capsule point to sphere center point */
		Hit* m = HitNext();
		m->contact_point = CapsuleClosestPoint(c, s.position);
		m->normal = sub3(s.position, m->contact_point);
		float d2 = dot3(m->normal, m->normal);
		if (d2 > s.radius * s.radius) return 0;

		/* normalize hit normal vector */
		float l = d2 != 0.0f ? sqrtf(d2) : 1;
		float linv = 1.0f / l;
		m->normal = scale3(m->normal, linv);

		/* calculate penetration depth */
		m->depth = d2 - s.radius * s.radius;
		m->depth = m->depth != 0.0f ? sqrtf(m->depth) : 0.0f;
		return m;
	}

	inline int CapsuleTestAABB(const Capsule& c, const AABB& a)
	{
		/* calculate aabb center point */
		vec3 ac = scale3(sub3(a.max, a.min), 0.5f);

		/* calculate closest point from aabb to point on capsule and check if inside aabb */
		vec3 p = CapsuleClosestPoint(c, ac);
		return AABBContainsPoint(a, p);
	}

	inline int CapsuleTestCapsule(const Capsule& a, const Capsule& b)
	{
		float t1, t2;
		vec3 c1, c2;
		float d2 = lineClosestLine_(&t1, &t2, &c1, &c2, Line(a.a, a.b), Line(b.a, b.b));
		float r = a.r + b.r;
		return d2 <= r * r;
	}

	inline int CapsuleTestPoly(const Capsule& c, const Poly& p)
	{
		return PolyTestCapsule(p, c);
	}

	inline int CapsuleTestSphere(const Capsule& c, const Sphere& s)
	{
		/* squared distance bwetween sphere center and capsule line segment */
		float d2 = LineDistance2Point(Line(c.a, c.b), s.position);
		float r = s.radius + c.r;
		return d2 <= r * r;
	}

	inline int PolyTestSphere(const Poly& p, const Sphere& s)
	{
		GJKResult res;
		return PolyHitSphere(&res, p, s);
	}

	inline int PolyTestAABB(const Poly& p, const AABB& a)
	{
		GJKResult res;
		return PolyHitAABB(&res, p, a);
	}

	inline int PolyTestCapsule(const Poly& p, const Capsule& c)
	{
		GJKResult res;
		return PolyHitCapsule(&res, p, c);
	}

	inline int PolyTestPoly(const Poly& a, const Poly& b)
	{
		GJKResult res;
		return PolyHitPoly(&res, a, b);
	}

	inline int PolyTestSphereTransform(const Poly& p, vec3 pos3, const glm::mat3& rot33, const Sphere& s)
	{
		GJKResult res;
		return PolyHitSphereTransform(&res, p, pos3, rot33, s);
	}

	inline int PolyTestAABBTransform(const Poly& p, vec3 apos3, const glm::mat3& arot33, const AABB& a)
	{
		GJKResult res;
		return PolyHitAABBTransform(&res, p, apos3, arot33, a);
	}

	inline int PolyTestCapsuleTransform(const Poly& p, vec3 pos3, const glm::mat3& rot33, const Capsule& c)
	{
		GJKResult res;
		return PolyHitCapsuleTransform(&res, p, pos3, rot33, c);
	}

	inline int PolyTestPolyTransform(const Poly& a, vec3 apos3, const glm::mat3& arot33, const Poly& b, vec3 bpos3, const glm::mat3& brot33)
	{
		GJKResult res;
		return PolyHitPolyTransform(&res, a, apos3, arot33, b, bpos3, brot33);
	}

	inline int PolyHitSphere(GJKResult* res, const Poly& p, const Sphere& s)
	{
		/* initial guess */
		glm::vec3 d = { 0.0f, 0.0f, 0.0f };
		GJKSupport gs = { 0 };
		gs.a = p.verts[0];
		gs.b = s.position;
		d = sub3(gs.b, gs.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &gs, &d)) 
		{
			vec3 n = scale3(d, -1);
			gs.aid = PolySupport(&gs.a, n, p);
			d = sub3(gs.b, gs.a);
		}
		/* check distance between closest points */
		*res = GJKAnalyze(&gsx);
		return res->distance_squared <= s.radius * s.radius;
	}

	inline int PolyHitAABB(GJKResult* res, const Poly& p, const AABB& a)
	{
		Poly poly;
		poly.verts = {
			vec3(a.min.x, a.min.y, a.min.z),
			vec3(a.min.x, a.min.y, a.max.z),
			vec3(a.min.x, a.max.y, a.min.z),
			vec3(a.min.x, a.max.y, a.max.z),
			vec3(a.max.x, a.min.y, a.min.z),
			vec3(a.max.x, a.min.y, a.max.z),
			vec3(a.max.x, a.max.y, a.min.z),
			vec3(a.max.x, a.max.y, a.max.z),
		};
		poly.cnt = 8;
		return PolyHitPoly(res, p, poly);
	}

#define f3dot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define f3cpy(d,s) (d)[0]=(s)[0],(d)[1]=(s)[1],(d)[2]=(s)[2]
	static int line_support(float* support, const float* d, const float* a, const float* b)
	{
		int i = 0;
		if (f3dot(a, d) < f3dot(b, d)) {
			f3cpy(support, b); i = 1;
		}
		else f3cpy(support, a);
		return i;
	}
	static int polyhedron_support(float* support, const float* d, const float* verts, int cnt)
	{
		int imax = 0;
		float dmax = f3dot(verts, d);
		for (int i = 1; i < cnt; ++i) {
			/* find vertex with max dot product in direction d */
			float dot = f3dot(&verts[i * 3], d);
			if (dot < dmax) continue;
			imax = i, dmax = dot;
		} f3cpy(support, &verts[imax * 3]);
		return imax;
}

	inline int PolyHitCapsule(GJKResult* res, const Poly& poly, const Capsule& capsule)
	{
#if 0
		
		/* initial guess */
		GJKSupport s = { 0 };
		s.a = poly.verts[0];
		s.b = capsule.a;
		glm::vec3 d = sub3(s.b, s.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &s, &d)) 
		{
			vec3 n = scale3(d, -1);
			s.aid = PolySupport(&s.a, n, poly);
			s.bid = LineSupport(&s.b, d, capsule.a, capsule.b);
			d = sub3(s.b, s.a);
		}
		/* check distance between closest points */
		assert(gsx.iter < gsx.max_iter);
		*res = GJKAnalyze(&gsx);
		return res->distance_squared <= capsule.r * capsule.r;
#else
		/* initial guess */
		gjk_support s = { 0 };
		f3cpy(s.a, glm::value_ptr(poly.verts[0]));
		f3cpy(s.b, glm::value_ptr(capsule.a));

		/* run gjk algorithm */
		gjk_simplex gsx = { 0 };
		while (gjk(&gsx, &s)) {
			s.aid = polyhedron_support(s.a, s.da, glm::value_ptr(poly.verts[0]), poly.cnt);
			s.bid = line_support(s.b, s.db, glm::value_ptr(capsule.a), glm::value_ptr(capsule.b));
		}
		/* check distance between closest points */
		gjk_result res2;
		gjk_analyze(&res2, &gsx);
		gjk_quad(&res2, 0, capsule.r);

		res->distance_squared = res2.distance_squared;
		res->hit = res2.hit;
		res->iterations = res2.iterations;
		res->p0.x = res2.p0[0];
		res->p0.y = res2.p0[1];
		res->p0.z = res2.p0[2];

		res->p1.x = res2.p1[0];
		res->p1.y = res2.p1[1];
		res->p1.z = res2.p1[2];

		return res2.hit;
#endif
	}

	inline int PolyHitPoly(GJKResult* res, const Poly& a, const Poly& b)
	{
		/* initial guess */
		glm::vec3 d = { 0.0f, 0.0f, 0.0f };
		GJKSupport gs = { 0 };
		gs.a = a.verts[0];
		gs.b = b.verts[0];
		d = sub3(gs.b, gs.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &gs, &d))
		{
			vec3 n = scale3(d, -1);
			gs.aid = PolySupport(&gs.a, n, a);
			gs.bid = PolySupport(&gs.b, d, b);
			d = sub3(gs.b, gs.a);
		}
		*res = GJKAnalyze(&gsx);
		return gsx.hit;
	}

	inline int PolyHitSphereTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const Sphere& s)
	{
		/* initial guess */
		glm::vec3 d = { 0.0f, 0.0f, 0.0f };
		GJKSupport gs = { 0 };
		gs.a = p.verts[0];
		gs.b = s.position;
		transformS(&gs.a, glm::value_ptr(rot33), pos3);
		d = sub3(gs.b, gs.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &gs, &d)) 
		{
			vec3 n = scale3(d, -1);
			vec3 da; transformT(&da, n, glm::value_ptr(rot33), pos3);

			gs.aid = PolySupport(&gs.a, da, p);
			transformS(&gs.a, glm::value_ptr(rot33), pos3);
			d = sub3(gs.b, gs.a);
		}
		/* check distance between closest points */
		*res = GJKAnalyze(&gsx);
		return res->distance_squared <= s.radius * s.radius;
	}

	inline int PolyHitAABBTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const AABB& a)
	{
		vec3 zero = glm::vec3{ 0 };
		glm::mat3 id = { 
			{1,0,0},
			{0,1,0},
			{0,0,1} };

		Poly poly;
		poly.verts = 
		{
			vec3(a.min.x, a.min.y, a.min.z),
			vec3(a.min.x, a.min.y, a.max.z),
			vec3(a.min.x, a.max.y, a.min.z),
			vec3(a.min.x, a.max.y, a.max.z),
			vec3(a.max.x, a.min.y, a.min.z),
			vec3(a.max.x, a.min.y, a.max.z),
			vec3(a.max.x, a.max.y, a.min.z),
			vec3(a.max.x, a.max.y, a.max.z),
		};
		poly.cnt = 8;

		return PolyHitPolyTransform(res, p, pos3, rot33, poly, zero, id);
	}

	inline int PolyHitCapsuleTransform(GJKResult* res, const Poly& p, vec3 pos3, const glm::mat3& rot33, const Capsule& c)
	{
		/* initial guess */
		glm::vec3 d = { 0.0f, 0.0f, 0.0f };
		GJKSupport gs = { 0 };
		gs.a = p.verts[0];
		gs.b = c.a;
		transformS(&gs.a, glm::value_ptr(rot33), pos3);
		d = sub3(gs.b, gs.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &gs, &d))
		{
			vec3 n = scale3(d, -1);
			vec3 da; transformT(&da, n, glm::value_ptr(rot33), pos3);

			gs.aid = PolySupport(&gs.a, da, p);
			gs.bid = LineSupport(&gs.b, d, c.a, c.b);
			transformS(&gs.a, glm::value_ptr(rot33), pos3);
			d = sub3(gs.b, gs.a);
		}
		/* check distance between closest points */
		*res = GJKAnalyze(&gsx);
		return res->distance_squared <= c.r * c.r;
	}

	inline int PolyHitPolyTransform(GJKResult* res, const Poly& a, vec3 at3, const glm::mat3& ar33, const Poly& b, vec3 bt3, const glm::mat3& br33)
	{
		/* initial guess */
		glm::vec3 d = { 0.0f, 0.0f, 0.0f };
		GJKSupport gs = { 0 };
		gs.a = a.verts[0];
		gs.b = b.verts[0];
		transformS(&gs.a, glm::value_ptr(ar33), at3);
		transformS(&gs.b, glm::value_ptr(br33), bt3);
		d = sub3(gs.b, gs.a);

		/* run gjk algorithm */
		GJKSimplex gsx = { 0 };
		while (GJK(&gsx, &gs, &d)) 
		{
			/* transform direction */
			vec3 n = scale3(d, -1);
			vec3 da; transformT(&da, n, glm::value_ptr(ar33), at3);
			vec3 db; transformT(&db, d, glm::value_ptr(br33), bt3);
			/* run support function on tranformed directions  */
			gs.aid = PolySupport(&gs.a, da, a);
			gs.bid = PolySupport(&gs.b, db, b);
			/* calculate distance vector on transformed points */
			transformS(&gs.a, glm::value_ptr(ar33), at3);
			transformS(&gs.b, glm::value_ptr(br33), bt3);
			d = sub3(gs.b, gs.a);
		}
		*res = GJKAnalyze(&gsx);
		return gsx.hit;
	}

	inline vec4  vec34(vec3 a, float w) { return vec4(a.x, a.y, a.z, w); }

	inline vec4 Plane4(vec3 p, vec3 n)
	{
		return vec34(n, -dot3(n, p));
	}

	inline Frustum FrustumBuild(const glm::mat4& pv)
	{
		Frustum f;
		// TODO: починить
		//f.l = vec4(pv[3] + pv[0], pv[7] + pv[4], pv[11] + pv[8], pv[15] + pv[12]);
		//f.r = vec4(pv[3] - pv[0], pv[7] - pv[4], pv[11] - pv[8], pv[15] - pv[12]);
		//f.t = vec4(pv[3] - pv[1], pv[7] - pv[5], pv[11] - pv[9], pv[15] - pv[13]);
		//f.b = vec4(pv[3] + pv[1], pv[7] + pv[5], pv[11] + pv[9], pv[15] + pv[13]);
		//f.n = vec4(pv[3] + pv[2], pv[7] + pv[6], pv[11] + pv[10], pv[15] + pv[14]);
		//f.f = vec4(pv[3] - pv[2], pv[7] - pv[6], pv[11] - pv[10], pv[15] - pv[14]);
		//for (int i = 0; i < 6; i++) 
		//	f.pl[i] = div4(f.pl[i], len3({ f.pl[i].x, f.pl[i].y, f.pl[i].z }));
		return f;
	}

	inline int FrustumTestSphere(const Frustum& f, const Sphere& s)
	{
		for (int i = 0; i < 6; i++)
		{
			if ((dot3({ f.pl[i].x, f.pl[i].y, f.pl[i].z }, s.position) + f.pl[i].w + s.radius) < 0) return 0;
		}
		return 1;
	}

	inline int FrustumTestAABB(const Frustum& f, const AABB& a)
	{
		for (int i = 0; i < 6; i++) 
		{
			vec3 v = vec3(f.pl[i].x > 0 ? a.max.x : a.min.x, f.pl[i].y > 0 ? a.max.y : a.min.y, f.pl[i].z > 0 ? a.max.z : a.min.z);
			if ((dot3({ f.pl[i].x, f.pl[i].y, f.pl[i].z }, v) + f.pl[i].w) < 0) return 0;
		}
		return 1;
	}

	inline Poly Pyramid(const glm::vec3& from, const glm::vec3& to, float size)
	{
		/* calculate axis */
		vec3 up, right, forward = norm3(sub3(to, from));
		ortho3(&right, &up, forward);

		/* calculate extend */
		vec3 xext = scale3(right, size);
		vec3 yext = scale3(up, size);
		vec3 nxext = scale3(right, -size);
		vec3 nyext = scale3(up, -size);

		/* calculate base vertices */
		Poly p;
		p.verts = {
			add3(add3(from, xext), yext), /*a*/
			add3(add3(from, xext), nyext), /*b*/
			add3(add3(from, nxext), nyext), /*c*/
			add3(add3(from, nxext), yext), /*d*/
			to, /*r*/
			{} // empty
		};
		p.cnt = 5; /*+1 for diamond case*/ // array_resize(p.verts, 5+1); p.cnt = 5;

		return p;
	}

	inline Poly Diamond(const glm::vec3& from, const glm::vec3& to, float size)
	{
		vec3 mid = add3(from, scale3(sub3(to, from), 0.5f));
		Poly p = Pyramid(mid, to, size);
		p.verts[5] = from; p.cnt = 6;
		return p;
	}

}
#include "sysdep.h"
#include "mytritri.h"

namespace coldet
{

	Vector3D my_tri_tri_intersect(const Triangle& t1, const Triangle& t2)
	{
		Plane p1(t1.v1, t1.v2, t1.v3);
		int other_side = 0;
		{
			float f1 = p1.Classify(t2.v1);
			float f2 = p1.Classify(t2.v2);
			float f3 = p1.Classify(t2.v3);
			float f12 = f1 * f2;
			float f23 = f2 * f3;
			if (f12 > 0.0f && f23 > 0.0f) return Vector3D::Zero;
			other_side = (f12 < 0.0f ? (f23 < 0.0f ? 1 : 0) : 2);
		}
		Plane p2(t2.v1, t2.v2, t2.v3);
		Vector3D n12(p1.normal + p2.normal);
		TriangleDesc td2(t2, p2);
		const Vector3D& a2 = td2[other_side + 1];
		const Vector3D& b2 = td2[other_side];
		const Vector3D& c2 = td2[other_side + 2];
		float t21 = -(p1.d + p2.d + a2 * n12) / ((b2 - a2) * n12);
		TriangleDesc td1(t1, p1);
		Vector3D P21(a2 + t21 * (b2 - a2));
		if (td1.pointInTri(P21)) return P21;
		float t22 = -(p1.d + p2.d + c2 * n12) / ((b2 - c2) * n12);
		Vector3D P22(c2 + t22 * (b2 - c2));
		if (td1.pointInTri(P22)) return P22;

		{
			float f1 = p2.Classify(t1.v1);
			float f2 = p2.Classify(t1.v2);
			float f3 = p2.Classify(t1.v3);
			float f12 = f1 * f2;
			float f23 = f2 * f3;
			if (f12 > 0.0f && f23 > 0.0f) return Vector3D::Zero;
			other_side = (f12 < 0.0f ? (f23 < 0.0f ? 1 : 0) : 2);
		}
		const Vector3D& a1 = td1[other_side + 1];
		const Vector3D& b1 = td1[other_side];
		const Vector3D& c1 = td1[other_side + 2];
		float t11 = -(p1.d + p2.d + a1 * n12) / ((b1 - a1) * n12);
		Vector3D P11(a1 + t11 * (b1 - a1));
		if (td2.pointInTri(P11)) return P11;
		float t12 = -(p1.d + p2.d + c1 * n12) / ((b1 - c1) * n12);
		Vector3D P12(c1 + t12 * (b1 - c1));
		if (td2.pointInTri(P12)) return P12;
		return Vector3D::Zero;
	}

} // namespace coldet
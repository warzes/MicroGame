#include "sysdep.h"
#include "coldet.h"
#include "mytritri.h"
#include <assert.h>

namespace coldet
{

	class Check
	{
	public:
		Check() {}
		Check(BoxTreeNode* f, BoxTreeNode* s, int d)
			: m_first(f), m_second(s), depth(d) {}
		BoxTreeNode* m_first;
		BoxTreeNode* m_second;
		int depth;
	};

	bool CollisionModel3D::collision(CollisionModel3D* other, int AccuracyDepth, int MaxProcessingTime, float* other_transform)
	{
		m_ColType = Models;
		if (!m_Final) throw Inconsistency();
		if (!other->m_Final) throw Inconsistency();
		Matrix3D t = (other_transform == NULL ? other->m_Transform : *((Matrix3D*)other_transform));
		if (m_Static) t *= m_InvTransform;
		else          t *= m_Transform.Inverse();
		RotationState rs(t);

		if (AccuracyDepth < 0) AccuracyDepth = 0xFFFFFF;
		if (MaxProcessingTime == 0) MaxProcessingTime = 0xFFFFFF;

		unsigned EndTime, BeginTime = unsigned(get_tick_count());
		int num = Max(m_Triangles.size(), other->m_Triangles.size());
		int Allocated = Max(64, (num >> 4));
		std::vector<Check> checks(Allocated);

		int queue_idx = 1;
		Check& c = checks[0];
		c.m_first = &m_Root;
		c.depth = 0;
		c.m_second = &other->m_Root;
		while (queue_idx > 0)
		{
			if (queue_idx > (Allocated / 2)) // enlarge the queue.
			{
				Check c;
				checks.insert(checks.end(), Allocated, c);
				Allocated *= 2;
			}
			EndTime = unsigned(get_tick_count());
			if (EndTime >= (BeginTime + MaxProcessingTime)) throw TimeoutExpired();

			// @@@ add depth check
			//Check c=checks.back();
			Check& c = checks[--queue_idx];
			BoxTreeNode* first = c.m_first;
			BoxTreeNode* second = c.m_second;
			assert(first != NULL);
			assert(second != NULL);
			if (first->intersect(*second, rs))
			{
				int tnum1 = first->getTrianglesNumber();
				int tnum2 = second->getTrianglesNumber();
				if (tnum1 > 0 && tnum2 > 0)
				{
					{
						for (int i = 0; i < tnum2; i++)
						{
							BoxedTriangle* bt2 = second->getTriangle(i);
							Triangle tt(Transform(bt2->v1, rs.t), Transform(bt2->v2, rs.t), Transform(bt2->v3, rs.t));
							for (int j = 0; j < tnum1; j++)
							{
								BoxedTriangle* bt1 = first->getTriangle(j);
								if (tt.intersect(*bt1))
								{
									m_ColTri1 = *bt1;
									m_iColTri1 = getTriangleIndex(bt1);
									m_ColTri2 = tt;
									m_iColTri2 = other->getTriangleIndex(bt2);
									return true;
								}
							}
						}
					}
				}
				else
					if (first->getSonsNumber() == 0)
					{
						BoxTreeNode* s1 = second->getSon(0);
						BoxTreeNode* s2 = second->getSon(1);
						assert(s1 != NULL);
						assert(s2 != NULL);

						Check& c1 = checks[queue_idx++];
						c1.m_first = first;
						c1.m_second = s1;

						Check& c2 = checks[queue_idx++];
						c2.m_first = first;
						c2.m_second = s2;
					}
					else
						if (second->getSonsNumber() == 0)
						{
							BoxTreeNode* f1 = first->getSon(0);
							BoxTreeNode* f2 = first->getSon(1);
							assert(f1 != NULL);
							assert(f2 != NULL);

							Check& c1 = checks[queue_idx++];
							c1.m_first = f1;
							c1.m_second = second;

							Check& c2 = checks[queue_idx++];
							c2.m_first = f2;
							c2.m_second = second;
						}
						else
						{
							float v1 = first->getVolume();
							float v2 = second->getVolume();
							if (v1 > v2)
							{
								BoxTreeNode* f1 = first->getSon(0);
								BoxTreeNode* f2 = first->getSon(1);
								assert(f1 != NULL);
								assert(f2 != NULL);

								Check& c1 = checks[queue_idx++];
								c1.m_first = f1;
								c1.m_second = second;

								Check& c2 = checks[queue_idx++];
								c2.m_first = f2;
								c2.m_second = second;
							}
							else
							{
								BoxTreeNode* s1 = second->getSon(0);
								BoxTreeNode* s2 = second->getSon(1);
								assert(s1 != NULL);
								assert(s2 != NULL);

								Check& c1 = checks[queue_idx++];
								c1.m_first = first;
								c1.m_second = s1;

								Check& c2 = checks[queue_idx++];
								c2.m_first = first;
								c2.m_second = s2;
							}
						}
			}
		}
		return false;
	}

	bool CollisionModel3D::rayCollision(const float origin[3],
		const float direction[3],
		bool closest,
		float segmin,
		float segmax)
	{
		float mintparm = 9e9f, tparm;
		Vector3D col_point;
		m_ColType = Ray;
		Vector3D O;
		Vector3D D;
		if (m_Static)
		{
			O = Transform(*(Vector3D*)origin, m_InvTransform);
			D = rotateVector(*(Vector3D*)direction, m_InvTransform);
		}
		else
		{
			Matrix3D inv = m_Transform.Inverse();
			O = Transform(*(Vector3D*)origin, inv);
			D = rotateVector(*(Vector3D*)direction, inv);
		}
		if (segmin != 0.0f) // normalize ray
		{
			O += segmin * D;
			segmax -= segmin;
			segmin = 0.0f;
		}
		if (segmax < segmin)
		{
			D = -D;
			segmax = -segmax;
		}
		std::vector<BoxTreeNode*> checks;
		checks.push_back(&m_Root);
		while (!checks.empty())
		{
			BoxTreeNode* b = checks.back();
			checks.pop_back();
			if (b->intersect(O, D, segmax))
			{
				int sons = b->getSonsNumber();
				if (sons)
					while (sons--) checks.push_back(b->getSon(sons));
				else
				{
					int tri = b->getTrianglesNumber();
					while (tri--)
					{
						BoxedTriangle* bt = b->getTriangle(tri);
						Triangle* t = static_cast<Triangle*>(bt);
						if (t->intersect(O, D, col_point, tparm, segmax))
						{
							if (closest)
							{
								if (tparm < mintparm)
								{
									mintparm = tparm;
									m_ColTri1 = *bt;
									m_iColTri1 = getTriangleIndex(bt);
									m_ColPoint = col_point;
								}
							}
							else
							{
								m_ColTri1 = *bt;
								m_iColTri1 = getTriangleIndex(bt);
								m_ColPoint = col_point;
								return true;
							}
						}
					}
				}
			}
		}
		if (closest && mintparm < 9e9f) return true;
		return false;
	}

	bool CollisionModel3D::sphereCollision(const float origin[3], float radius)
	{
		m_ColType = Sphere;
		Vector3D O;
		if (m_Static)
			O = Transform(*(Vector3D*)origin, m_InvTransform);
		else
		{
			Matrix3D inv = m_Transform.Inverse();
			O = Transform(*(Vector3D*)origin, inv);
		}
		std::vector<BoxTreeNode*> checks;
		checks.push_back(&m_Root);
		while (!checks.empty())
		{
			BoxTreeNode* b = checks.back();
			checks.pop_back();
			if (b->intersect(O, radius))
			{
				int sons = b->getSonsNumber();
				if (sons)
					while (sons--) checks.push_back(b->getSon(sons));
				else
				{
					int tri = b->getTrianglesNumber();
					while (tri--)
					{
						BoxedTriangle* bt = b->getTriangle(tri);
						Triangle* t = static_cast<Triangle*>(bt);
						if (t->intersect(O, radius, m_ColPoint))
						{
							m_ColTri1 = *bt;
							m_iColTri1 = getTriangleIndex(bt);
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool CollisionModel3D::getCollidingTriangles(float t1[9], float t2[9], bool ModelSpace)
	{
		if (ModelSpace)
		{
			if (t1 != NULL)
			{
				*((Vector3D*)&t1[0]) = m_ColTri1.v1;
				*((Vector3D*)&t1[3]) = m_ColTri1.v2;
				*((Vector3D*)&t1[6]) = m_ColTri1.v3;
			}
			if (t2 != NULL)
			{
				*((Vector3D*)&t2[0]) = m_ColTri2.v1;
				*((Vector3D*)&t2[3]) = m_ColTri2.v2;
				*((Vector3D*)&t2[6]) = m_ColTri2.v3;
			}
		}
		else
		{
			if (t1 != NULL)
			{
				*((Vector3D*)&t1[0]) = Transform(m_ColTri1.v1, m_Transform);
				*((Vector3D*)&t1[3]) = Transform(m_ColTri1.v2, m_Transform);
				*((Vector3D*)&t1[6]) = Transform(m_ColTri1.v3, m_Transform);
			}
			if (t2 != NULL)
			{
				*((Vector3D*)&t2[0]) = Transform(m_ColTri2.v1, m_Transform);
				*((Vector3D*)&t2[3]) = Transform(m_ColTri2.v2, m_Transform);
				*((Vector3D*)&t2[6]) = Transform(m_ColTri2.v3, m_Transform);
			}
		}
		return true;
	}

	bool CollisionModel3D::getCollidingTriangles(int& t1, int& t2)
	{
		t1 = m_iColTri1;
		t2 = m_iColTri2;
		return true;
	}

	bool CollisionModel3D::getCollisionPoint(float p[3], bool ModelSpace)
	{
		Vector3D& v = *((Vector3D*)p);
		switch (m_ColType)
		{
		case Models: v = my_tri_tri_intersect(m_ColTri1, m_ColTri2); break;
		case Sphere:
		case Ray:    v = m_ColPoint; break;
		default:     v = Vector3D::Zero;
		}
		if (!ModelSpace) v = Transform(v, m_Transform);
		return true;
	}


	bool SphereRayCollision(const float* center, float radius,
		const float* origin, const float* direction,
		float* point)
	{
		const Vector3D& C = *((const Vector3D*)center);
		const Vector3D& O = *((const Vector3D*)origin);
		const Vector3D  D = ((const Vector3D*)direction)->Normalized();
		Vector3D& P = *((Vector3D*)point);
		Vector3D EO = C - O;
		float v = EO * D;
		float disc = radius * radius - (EO * EO - v * v);
		if (disc < 0.0f) return false;
		float d = sqrt(disc);
		P = O + (v - d) * D;
		return true;
	}

	bool SphereSphereCollision(float c1[3], float r1,
		float c2[3], float r2,
		float point[3])
	{
		Vector3D& C1 = *((Vector3D*)c1);
		Vector3D& C2 = *((Vector3D*)c2);
		float dist = (C2 - C1).SquareMagnitude();
		float sum = r1 + r2;
		if (dist < sum * sum)
		{
			Vector3D& P = *((Vector3D*)point);

			P = C1 - C2;
			P.Normalized();
			P *= r1;
			P += C1;
			return true;
		}
		return false;
	}

	EXPORT CollisionModel3D* newCollisionModel3D(bool Static)
	{
		return new CollisionModel3D(Static);
	}

	CollisionModel3D::CollisionModel3D(bool Static)
		: m_Root(Vector3D::Zero, Vector3D::Zero, 0),
		m_Transform(Matrix3D::Identity),
		m_InvTransform(Matrix3D::Identity),
		m_ColTri1(Vector3D::Zero, Vector3D::Zero, Vector3D::Zero),
		m_ColTri2(Vector3D::Zero, Vector3D::Zero, Vector3D::Zero),
		m_iColTri1(0),
		m_iColTri2(0),
		m_Final(false),
		m_Static(Static),
		m_Radius(0)
	{}

	void CollisionModel3D::addTriangle(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3)
	{
		if (m_Final) throw Inconsistency();
		m_Triangles.push_back(BoxedTriangle(v1, v2, v3));
		m_Radius = Max(m_Radius, v1.SquareMagnitude());
		m_Radius = Max(m_Radius, v2.SquareMagnitude());
		m_Radius = Max(m_Radius, v3.SquareMagnitude());
	}

	void CollisionModel3D::setTransform(const Matrix3D& m)
	{
		m_Transform = m;
		if (m_Static) m_InvTransform = m_Transform.Inverse();
	}

	void CollisionModel3D::finalize()
	{
		if (m_Final) throw Inconsistency();
		// Prepare initial triangle list
		m_Final = true;
		m_Radius = sqrt(m_Radius);
		for (unsigned i = 0; i < m_Triangles.size(); i++)
		{
			BoxedTriangle& bt = m_Triangles[i];
			m_Root.m_Boxes.push_back(&bt);
		}
		int logdepth = 0;
		for (int num = m_Triangles.size(); num > 0; num >>= 1, logdepth++);
		m_Root.m_logdepth = int(logdepth * 1.5f);
		m_Root.divide(0);
	}

} // namespace coldet
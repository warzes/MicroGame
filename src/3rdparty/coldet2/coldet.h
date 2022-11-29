#pragma once

#include "box.h"
#include "cdmath3d.h"
#include <vector>

namespace coldet
{
#define EXPORT

	/** Collision Model.  Will represent the mesh to be tested for
	collisions.  It has to be notified of all triangles, via
	addTriangle()
	After all triangles are added, a call to finalize() will
	process the information and prepare for collision tests.
	Call collision() to check for a collision

	Note: Transformations must not contain scaling.
	*/
	class CollisionModel3D
	{
	public:
		CollisionModel3D(bool Static);

		/** Optional: Optimization for construction speed.
		If you know the number of triangles. */
		void setTriangleNumber(int num) { if (!m_Final) m_Triangles.reserve(num); }

		/** Use any of the forms of this functions to enter the coordinates
		of the model's triangles. */
		void addTriangle(float x1, float y1, float z1,
			float x2, float y2, float z2,
			float x3, float y3, float z3)
		{
			addTriangle(
				Vector3D(x1, y1, z1),
				Vector3D(x2, y2, z2),
				Vector3D(x3, y3, z3));
		}
		void addTriangle(const float v1[3], const float v2[3], const float v3[3])
		{
			addTriangle(
				Vector3D(v1[0], v1[1], v1[2]),
				Vector3D(v2[0], v2[1], v2[2]),
				Vector3D(v3[0], v3[1], v3[2]));
		}
		void addTriangle(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);

		/** All triangles have been added, process model. */
		void finalize();

		/** Returns the bounding sphere radius
		Note that this is not the optimal bounding sphere, but centered
		in the origin of the coordinate system of the triangles */
		float getRadius() { return m_Radius; }

		/** The the current affine matrix for the model.
		See transform.txt for format information */
		void setTransform(const float m[16]) { setTransform(*(Matrix3D*)m); }
		void setTransform(const Matrix3D& m);

		/** Check for collision with another model.
		Do not mix model types here.

		MaxProcessingTime determines the maximum time in milliseconds
		to check for collision.  If a rejection is not found by that
		time, the function will return true.

		AccuracyDepth is not yet supported.

		other_transform allows overriding the other model's
		transform, by supplying an alternative one.
		This can be useful when testing a model against itself
		with different orientations.
		*/
		bool collision(CollisionModel3D* other,
			int AccuracyDepth = -1,
			int MaxProcessingTime = 0,
			float* other_transform = 0);

		/** Returns true if the ray given in world space coordinates
		intersects with the object.
		getCollidingTriangles() and getCollisionPoint() can be
		used to retrieve information about a collision.
		If closest if false, the first triangle that collides with
		the ray is used.  Otherwise the closest one will be used.
		Closest triangle searching will slow the test considerably.
		The default ray is a standard infinite ray.  However, using
		segmin and segmax you can define a line segment along the
		ray.
		*/
		bool rayCollision(const float origin[3],
			const float direction[3],
			bool closest = false,
			float segmin = 0.0f,
			float segmax = 3.4e+38F);;

		/** Returns true if the given sphere collides with the model.
		getCollidingTriangles() and getCollisionPoint() can be
		used to retrieve information about a collision.
		*/
		bool sphereCollision(const float origin[3], float radius);

		/** Retrieve the pair of triangles that collided.
		Only valid after a call to collision() that returned true.
		t1 is this model's triangle and t2 is the other one.
		In case of ray or sphere collision, only t1 will be valid.
		The coordinates will be in _this_ model's coordinate space,
		unless ModelSpace is false, in which case, coordinates will
		be transformed by the model's current transform to world space.
		*/
		bool getCollidingTriangles(float t1[9], float t2[9], bool ModelSpace = true);

		/** Retrieve the pair of triangles indices that collided.
		Only valid after a call to collision() that returned true.
		t1 belongs to _this_ model, while t2 is in the other one.
		*/
		bool getCollidingTriangles(int& t1, int& t2);

		/** Retrieve the detected collision point.
		Only valid after a call to collision()
		that returned true.
		The coordinates will be in _this_ model's coordinate space,
		unless ModelSpace is false, in which case, coordinates will
		be transformed by the model's current transform to world space.
		*/
		bool getCollisionPoint(float p[3], bool ModelSpace = true);

		int getTriangleIndex(BoxedTriangle* bt)
		{
			return int(bt - &(*m_Triangles.begin()));
		}

	private:
		/** Stores all the actual triangles.  Other objects will use
		pointers into this array.
		*/
		std::vector<BoxedTriangle> m_Triangles;
		/** Root of the hierarchy tree */
		BoxTreeInnerNode           m_Root;
		/** The current transform and its inverse */
		Matrix3D                   m_Transform, m_InvTransform;
		/** The triangles that last collided */
		Triangle                   m_ColTri1, m_ColTri2;
		/** The indices of the triangles that last collided */
		int                        m_iColTri1, m_iColTri2;
		/** The collision point of the last test */
		Vector3D                   m_ColPoint;

		/** Type of the last collision test */
		enum { Models, Ray, Sphere }
		m_ColType;
		/** Flag for indicating the model is finalized. */
		bool                       m_Final;
		/** Static models will maintain the same transform for a while
		so the inverse transform is calculated each set instead
		of in the collision test. */
		bool                       m_Static;

		float m_Radius;
	};

	/** Timeout exception class.  Exception will be thrown if
	the detection algorithm could not complete within
	the given time limit. */
	class TimeoutExpired {};

	/** Inconsistency exception. Exception will be thrown if
	the model is inconsistent.
	Examples:
	Checking for collisions before calling finalize()
	Trying to add triangles after calling finalize()  */
	class Inconsistency {};

	/** Create a new collision model object.
	Use delete when finished with it.

	Setting Static to true indicates that the model does not
	move a lot, and certain calculations can be done every time
	its transform changes instead of every collision test.
	*/
	EXPORT CollisionModel3D* newCollisionModel3D(bool Static = false);



	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////

	/** Checks for intersection between a ray and a sphere.
	center, radius define the sphere
	origin, direction define the ray
	point will contain point of intersection, if one is found.
	*/
	EXPORT bool SphereRayCollision(const float* center, float radius,
		const float* origin, const float* direction,
		float* point);

	/** Checks for intersection between 2 spheres. */
	EXPORT bool SphereSphereCollision(float c1[3], float r1, float c2[3], float r2, float point[3]);

} // namespace coldet

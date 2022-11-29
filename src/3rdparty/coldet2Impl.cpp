#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4365)
#	pragma warning(disable : 5039)
#	pragma warning(disable : 5045)
#	pragma warning(disable : 5262)
#	pragma warning(disable : 5264)
#endif

/*
Quickstart:

Model Setup:
	For each mesh, create a collision model by using:
		CollisionModel3D* model = newCollisionModel3D();
	(Shared meshes can use one model)
	Add all the triangles the mesh has to the model by using:
	model->addTriangle(vertex1,vertex2,vertex3);
	Call:
		model->finalize();

Collision Test:
	Assuming you have two models (m1,m2), either set both of theirТs transformation
	matrices (world matrix) by calling:
		m1->setTransform(model1_transformation_matrix);
		m2->setTransform(model2_transformation_matrix);
	or set only one of them (in case youТre testing the model against itself, with different
	transform) Then call:
	m1->collision(m2);
	The function returns a bool indicating if a collision has occurred.   Note that if you test a
	model against itself with a different transform, you need to supply that transform as an
	optional parameter.

Collision Test Results:
	Use the getCollidingTriangles function to get which triangles have collided.   Use the
	getCollisionPoint function to find the exact collision point.

Other Collision Tests:
	You can use the rayCollision and sphereCollision functions to test the model against
	these primitives.
*/

// TODO: избавитьс€ от виртуального наследовани€
// переписать Dot функцию вместо operator*

#include <coldet2/box.cpp>
#include <coldet2/box_bld.cpp>
#include <coldet2/cdmath3d.cpp>
#include <coldet2/coldet.cpp>
#include <coldet2/multiobject.cpp>
#include <coldet2/mytritri.cpp>
#include <coldet2/sysdep.cpp>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
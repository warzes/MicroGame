#pragma once

#include "coldet.h"
#include "sysdep.h"

namespace coldet
{
	class TransformUpdater
	{
	public:
		virtual ~TransformUpdater() {}
		virtual const float* update() const = 0;
	};

	struct CollisionDetails
	{
		int   id1, id2;
		float point[3];
		float t1[9], t2[9];
	};

	/** Generic class to handle multiple objects scenes */
	class MultiObjectSystem
	{
	public:
		virtual ~MultiObjectSystem() {}

		/** Adds a model to the system at a specific position.  model is not owned by the system
		Returns a unique ID of the model, for later move / remove */
		virtual int add_object(CollisionModel3D* model, const float* position) = 0;

		/** Adds a model to the system at a specific position.  model is not owned by the system
		Returns a unique ID of the model, for later move / remove */
		virtual int add_object(CollisionModel3D* model, const TransformUpdater* updater) = 0;

		/** Adds an object that is not composed of polygons, but can be
		represented by a sphere.
		Returns a unique ID of the model, for later move / remove */
		virtual int add_sphere_object(float radius, const float* position) = 0;

		/** Remove model from the system.
		'id' is the value returned when the model was added */
		virtual void remove_object(int id) = 0;

		/** Move the model in space.  Will incrementally update the system.
		'id' is the value returned when the model was added
		'position' is the new position (3D vector)   */
		virtual void move_object(int id, const float* new_position) = 0;

		/** Retrieve collision model from id */
		virtual CollisionModel3D* get_collision_model(int id) = 0;

		/** Check for collisions between all pairs.
		'exact' indicates activation of triangle accurate intersection test
		without which, collisions are only estimated by proximity
		Returns the number of collisions found */
		virtual int check_collisions(bool exact) = 0;

		/** Retrieve collision details.
		'index' is between 0..(N-1)  N was the value returned from check_collisions
		id1,id2 are the two colliding models.  If exact collision was used,
		the collision details can be retrieved by supplying the details struct */
		virtual bool get_collision(int index, CollisionDetails& details) = 0;
	};

	MultiObjectSystem* newSpheresSystem();
	MultiObjectSystem* newSweepPruneSystem(int max_objects);

} // namespace coldet
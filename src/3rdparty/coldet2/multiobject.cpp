#include "multi_impl.h"

namespace coldet
{

	MultiObjectSystem* newSpheresSystem()
	{
		return new SphereSystem();
	}

	MultiObjectSystem* newSweepPruneSystem(int max_objects)
	{
		return new SweepPruneSystem(max_objects);
	}

} // namespace coldet
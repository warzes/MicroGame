#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4701)
#	pragma warning(disable : 5039)
#	pragma warning(disable : 5045)
#	pragma warning(disable : 5264)
#endif

// LinearMath

#include "LinearMath/btAlignedAllocator.cpp"
#include "LinearMath/btGeometryUtil.cpp"
#include "LinearMath/btSerializer.cpp"
#include "LinearMath/btVector3.cpp"
#include "LinearMath/btConvexHull.cpp"
#include "LinearMath/btPolarDecomposition.cpp"
#include "LinearMath/btSerializer64.cpp"
#include "LinearMath/btConvexHullComputer.cpp"
#include "LinearMath/btQuickprof.cpp"
#include "LinearMath/btThreads.cpp"
#include "LinearMath/btReducedVector.cpp"
#include "LinearMath/TaskScheduler/btTaskScheduler.cpp"
#include "LinearMath/TaskScheduler/btThreadSupportPosix.cpp"
#include "LinearMath/TaskScheduler/btThreadSupportWin32.cpp"

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
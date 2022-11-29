#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4365)
#	pragma warning(disable : 5039)
#	pragma warning(disable : 5045)
#	pragma warning(disable : 5262)
#	pragma warning(disable : 5264)
#endif

// TODO: избавиться от наследования от CollisionModel3D и MultiObjectSystem
// переписать Dot функцию

#include <coldet2/box.cpp>
#include <coldet2/box_bld.cpp>
#include <coldet2/cdmath3d.cpp>
#include <coldet2/coldet.cpp>
#include <coldet2/coldet_bld.cpp>
#include <coldet2/multiobject.cpp>
#include <coldet2/mytritri.cpp>
#include <coldet2/sysdep.cpp>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
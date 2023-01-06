#include "stdafx.h"
#include "World.h"
#include "Streams.h"

bool load_world(const char* mname, const char* cname)
{
	auto loadingstart = std::chrono::high_resolution_clock::now();

	std::string ogzname = "../IsekaiWorldData/map/TestMyDnDSandstorm.ogz";

	stream* f = opengzfile(ogzname.c_str(), "rb");

	return false;
}
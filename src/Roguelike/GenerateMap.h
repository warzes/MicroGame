#pragma once

#include "World.h"

namespace GenerateMap
{
	void GenerateDungeons(Map& map);

	glm::vec2 GetFindPosition(const Map& map);
}
#pragma once

#include "EngineConfig.h"

//=============================================================================
// Disable Compile Warning
//=============================================================================

#if defined(_MSC_VER)
#	pragma warning(disable : 4514)
#	pragma warning(disable : 4820)
#	pragma warning(push, 0)
#	pragma warning(disable : 5262)
#	pragma warning(disable : 5264)
#endif

//=============================================================================
// Platform Header
//=============================================================================

#if defined(_WIN32)
#	include <io.h>
#endif

#if defined(__linux__)
#endif

#if defined(__EMSCRIPTEN__)
#endif

//=============================================================================
// STL Header
//=============================================================================

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include <random>

//=============================================================================
// 3rdparty Header
//=============================================================================

#include <glfwMinimalHeader.h>
#include <glad/gl.h>

#include <stb/stb_truetype.h>
#include <stb/stb_image.h>
#include <tiny_obj_loader.h>

/*
Left handed
	Y   Z
	|  /
	| /
	|/___X
*/
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_INLINE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_SILENT_WARNINGS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/normal.hpp>

#include <GJK/gjk.h>

//=============================================================================
// Default Compile Warning
//=============================================================================
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
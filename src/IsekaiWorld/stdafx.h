#pragma once

#pragma warning(disable : 5045)
#pragma warning(push, 0) // delete

//#define STDAFX_INCLUDE

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <algorithm>

#include <zlib.h>
#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

#include <glad/gl.h>
#include "Engine.h"

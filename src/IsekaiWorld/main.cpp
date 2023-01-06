﻿#include "stdafx.h"
#include "World.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "MicroEngine.lib" )
#endif
//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	if (engine::CreateEngine({}))
	{
		load_world("TestMyDnDSandstorm");




		while (engine::IsRunningEngine())
		{
			const float deltaTime = engine::GetDeltaTime();
			engine::BeginFrameEngine();

			engine::EndFrameEngine();
		}
	}

	engine::DestroyEngine();
}
//-----------------------------------------------------------------------------
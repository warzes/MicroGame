#include "stdafx.h"
#include "Engine.h"
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	if (engine::CreateEngine({}))
	{
		while (engine::IsRunningEngine())
		{
			const float deltaTime = engine::GetDeltaTime();
			engine::BeginFrameEngine();

			engine::EndFrameEngine();
		}
	}

	engine::DestroyEngine();
}
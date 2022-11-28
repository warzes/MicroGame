#include "stdafx.h"
#include "Engine.h"
#include "GameApp.h"
//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	if (engine::CreateEngine({}))
	{
		if (StartGameApp())
		{
			while (engine::IsRunningEngine())
			{
				const float deltaTime = engine::GetDeltaTime();

				UpdateGameApp(deltaTime);

				engine::BeginFrameEngine();
				FrameGameApp(deltaTime);
				engine::EndFrameEngine();
			}
		}
		CloseGameApp();
	}

	engine::DestroyEngine();
}
//-----------------------------------------------------------------------------
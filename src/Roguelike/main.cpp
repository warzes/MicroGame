#include "stdafx.h"
#include "GameApp.h"
#if defined(_MSC_VER)
#	pragma comment( lib, "MicroEngine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif
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
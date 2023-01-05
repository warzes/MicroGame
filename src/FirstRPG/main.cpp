#include "stdafx.h"
#include "Engine.h"
#if defined(_MSC_VER)
#	pragma comment( lib, "MicroEngine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif
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
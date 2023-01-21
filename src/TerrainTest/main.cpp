#include "stdafx.h"
#include "GameApp.h"
#if defined(_MSC_VER)
#	pragma comment( lib, "MicroEngine.lib" )
#endif
//https://github.com/jing-interactive/melo
//
//та демка с пальмами
//https://trederia.blogspot.com/2014/09/water-in-opengl-and-gles-20-part-1.html
//https://github.com/AndrewMeshekoff/Portfolio/tree/master/Graphics/Procedural%20Islands
//https://www.youtube.com/watch?v=Gj4vvlWuL8E
//https://alain.xyz/libraries/crosswindow
//https://www.youtube.com/watch?v=qgDPSnZPGMA&t=16s
//https://github.com/CobaltXII/Tempest
//https://github.com/realkushagrakhare/ProjectWater
//https://github.com/zifeo/Infinite-terrain
//https://github.com/damdoy/opengl_terrain
//https://github.com/arthurdouillard/water_simulation
//https://github.com/stanfortonski/Procedural-Terrain-Generator-OpenGL
//https://github.com/stanfortonski/Ocean-OpenGL
//retro3d

//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	engine::EngineCreateInfo createInfo;
	createInfo.Window.Width = 1280;
	createInfo.Window.Height = 720;
	if (engine::CreateEngine(createInfo))
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
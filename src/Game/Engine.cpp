#include "stdafx.h"
#include "Engine.h"
//=============================================================================
#if defined(_MSC_VER)
//#	pragma comment( lib, "3rdparty.lib" )
#endif
//=============================================================================
#if defined(_WIN32)
extern "C"
{
	// NVIDIA: Force usage of NVidia GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> See "Enabling High Performance Graphics Rendering on Optimus Systems" http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

	// AMD: Force usage of AMD GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> Named "Dynamic Switchable Graphics", found no official documentation, only https://community.amd.com/message/1307599#comment-1307599 - "Can an OpenGL app default to the discrete GPU on an Enduro system?"
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // _WIN32
//=============================================================================
namespace engine
{
	namespace
	{
		constexpr static float MicrosecondsToSeconds = 1.0f / 1000000.0f;
		std::chrono::steady_clock::time_point startTime;
		int64_t frameTimeCurrent = 0;
		int64_t frameTimeLast = 0;
		int64_t frameTimeDelta = 0;
		float deltaTime = 0.0f;
		bool IsEngineExit = true;
	}


	bool CreateEngine(const EngineCreateInfo& createInfo)
	{
		if (!CreateLogSystem(createInfo.Log))
			return false;
		
		if (!CreateWindowSystem(createInfo.Window))
			return false;

		if (!CreateRenderSystem(createInfo.Render))
			return false;

		startTime = std::chrono::high_resolution_clock::now();

		IsEngineExit = false;

		return true;
	}
	void DestroyEngine()
	{
		IsEngineExit = true;
		g3d::ModelFileManager::Destroy();
		TextureFileManager::Destroy();
		DestroyRenderSystem();
		DestroyWindowSystem();
		DestroyLogSystem();
	}

	bool IsRunningEngine()
	{
		return IsEngineExit || !WindowShouldClose();
	}
	void BeginFrameEngine()
	{
		// get delta time
		{
			const auto curTime = std::chrono::high_resolution_clock::now();
			frameTimeCurrent = std::chrono::duration_cast<std::chrono::microseconds>(curTime - startTime).count();
			frameTimeDelta = frameTimeCurrent - frameTimeLast;
			frameTimeLast = frameTimeCurrent;
			deltaTime = static_cast<float>(frameTimeDelta) * MicrosecondsToSeconds;
		}

		BeginRenderFrame();
	}
	void EndFrameEngine()
	{
		UpdateWindow();
		UpdateInput();
	}

	float GetDeltaTime()
	{
		return deltaTime;
	}
}
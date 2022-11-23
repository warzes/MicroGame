#include "stdafx.h"
#include "Engine.h"
//=============================================================================
#if defined(_MSC_VER)
//#	pragma comment( lib, "3rdparty.lib" )
#endif
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
		bool IsExitRequested = true;
	}

	bool CreateEngine(const EngineCreateInfo& createInfo)
	{
		if (!CreateLogSystem(createInfo.Log))
			return false;
		
		if (!CreateWindowSystem(createInfo.Window))
			return false;

		if (!RenderSystem::Create(createInfo.Render))
			return false;

#if USE_PHYSX5 || USE_BULLET
		if (!PhysicsSystem::Create())
			return false;
#endif

		startTime = std::chrono::high_resolution_clock::now();

		IsExitRequested = false;

		return true;
	}
	void DestroyEngine()
	{
		IsExitRequested = true;
		g3d::ModelFileManager::Destroy();
		ShaderLoader::Destroy();
		TextureLoader::Destroy();

#if USE_PHYSX5 || USE_BULLET
		PhysicsSystem::Destroy();
#endif
		RenderSystem::Destroy();
		DestroyWindowSystem();
		DestroyLogSystem();
	}

	bool IsRunningEngine()
	{
		return IsExitRequested || !WindowShouldClose();
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

#if USE_PHYSX5 || USE_BULLET
		PhysicsSystem::FixedUpdate(deltaTime);
#endif

		RenderSystem::BeginFrame();
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

	void ExitRequested()
	{
		IsExitRequested = true;
	}
}
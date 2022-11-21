#pragma once

/*
	TODO:
		- дефолтные рендерресы в рендерсистему
		- генератор геометрии в рендерсистему
		- в генераторе геометрии передавать видовую матрицу, а не камеру
		- рендерстейт
		- в сцену добавить варианты камер (то есть в рендере только набор данных, а в сцене управляющая типа от первого лица, полет и т.д.)
		- возможно функция вращения объекта в сторону другого - https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp
		- сделать возможность установки атрибутов в вао по типу (vec2, vec3, mat4, etc) чтобы не нужно было руками высчитывать размеры и отступы. (старый способ сохранить). не забыть это добавить в инстансинг
	G3D TODO:
		- в Model сейчас создается столько вао, сколько создано сабмешей. Нужно переделать под решение с одним вао и сдвигами по памяти через DrawElementsBaseVertex
*/

#include "EngineConfig.h"
#include "BaseHeader.h"
#include "Base.h"
#include "Core.h"
#include "Container.h"
#include "EngineMath.h"
#include "Collide.h"
#include "Utility.h"
#include "Platform.h"
#include "Audio.h"
#include "oRenderer.h"
#include "Graphics.h"
#include "Physics.h"
#include "UI.h"
#include "Navigation.h"
#include "Scene.h"
#include "World.h"

namespace engine
{
	struct EngineCreateInfo
	{
		LogCreateInfo Log;
		WindowCreateInfo Window;
		RenderSystem::CreateInfo Render;
	};

	bool CreateEngine(const EngineCreateInfo& createInfo);
	void DestroyEngine();

	bool IsRunningEngine();
	void BeginFrameEngine();
	void EndFrameEngine();

	float GetDeltaTime();

	void ExitRequested();
}
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

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"
#include "2_Base.h"
#include "3_Core.h"
#include "4_Math.h"
#include "5_Utility.h"
#include "6_Platform.h"
#include "7_Audio.h"
#include "8_Renderer.h"
#include "9_Graphics.h"
#include "10_Physics.h"
#include "11_UI.h"
#include "12_Navigation.h"
#include "13_Scene.h"
#include "14_World.h"

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
}
#pragma once

избавится от старой версии gjk
возможно написать враппер для gjk
еще пример с реализацией игрока - https://github.com/kevinmoran/GJK

https://github.com/kroitor/gjk.c

https://github.com/kevinmoran/Basic-Lighting
https://github.com/kevinmoran/AnimationPractice
https://github.com/kevinmoran/3D-Platformer-Project
https://github.com/kevinmoran/Heightmapped-Terrain

#include "6_Platform.h"
#include "8_oRenderer.h"

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTexCoord;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(vPos, 1.0);
	vTexCoord = aTexCoord;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;

uniform sampler2D uSampler;

out vec4 fragColor;

void main()
{
	vec4 textureClr = texture(uSampler, vTexCoord);
	if (textureClr.a < 0.02) discard;
	fragColor = textureClr;
}
)";

ShaderProgram shader;
UniformLocation worldUniform;
UniformLocation viewUniform;
UniformLocation projectionUniform;

g3d::Model model;
g3d::Material material;
Transform transform;

Camera ncamera;

Poly polyModel;


void InitTest()
{
	ncamera.Teleport(0, 5, -10);
	ncamera.LookAt(glm::vec3(0, 0, 0));

	ncamera.Enable();
	ncamera.m_speed = 1;


	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		worldUniform = shader.GetUniformVariable("uWorld");
		viewUniform = shader.GetUniformVariable("uView");
		projectionUniform = shader.GetUniformVariable("uProjection");
	}

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/crate.png");
	}

	// Load geometry
	{
		model.Create("../data/models/rock.obj");
		model.SetMaterial(material);

		polyModel = model.GetPoly();
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
	shader.Destroy();
}


// Capsule pos

glm::vec3 wasdec2 = glm::vec3(-4, 0, 0);
glm::vec3 oldwasdec2 = glm::vec3(-4, 0, 0);

void FrameTest(float deltaTime)
{
	bool active = IsMouseButtonDown(0);
	SetMouseLock(active);

	const float xpos = GetMouseX();
	const float ypos = GetMouseY();
	static float lastPosX = xpos;
	static float lastPosY = ypos;
	glm::vec2 mouse = tempMath::scale2(glm::vec2((lastPosX - xpos), (lastPosY - ypos)), 200.0f * deltaTime * active);
	lastPosX = xpos;
	lastPosY = ypos;
			
	glm::vec3 wasdec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_A) - IsKeyboardKeyDown(KEY_D), IsKeyboardKeyDown(KEY_E) - IsKeyboardKeyDown(KEY_C), IsKeyboardKeyDown(KEY_W) - IsKeyboardKeyDown(KEY_S)), ncamera.m_speed * deltaTime);

	wasdec2 += tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_L) - IsKeyboardKeyDown(KEY_J), IsKeyboardKeyDown(KEY_U) - IsKeyboardKeyDown(KEY_O), IsKeyboardKeyDown(KEY_I) - IsKeyboardKeyDown(KEY_K)), ncamera.m_speed * deltaTime);

	ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
	ncamera.Fps(mouse.x, mouse.y);

	std::string ss = std::to_string(ncamera.m_position.x) + "|" + std::to_string(ncamera.m_position.y) + "|" + std::to_string(ncamera.m_position.z);
	puts(ss.c_str());

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());
	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();

	//DebugDraw::DrawGrid(0);

	static int paused = 0;
	if (IsKeyboardKeyDown(KEY_SPACE)) paused ^= 1;

	static bool inverts = false;
	static float he = 0.0;
	if (inverts)
		he -= deltaTime * !paused;
	else
		he += deltaTime * !paused;

	if (he > 2)
		inverts = true;
	if (he < -2)
		inverts = false;

	unsigned rgbSel;
	

	// Poly-Capsule (GJK) intersection
	{

		//const float x = 0;
		//const float y = 1.0f * he;
		//const float z = 0;
		float x = wasdec2.x;
		float y = wasdec2.y;
		float z = wasdec2.z;


		Capsule c = Capsule(glm::vec3(x, y, z), glm::vec3(x, y + 0.5f, z), 0.2f);

		collide::GJKResult gjk;
		if (collide::PolyHitCapsule(&gjk, polyModel, c))
		{
			rgbSel = RED;
			wasdec2 = oldwasdec2;
		}			
		else
		{
			oldwasdec2 = wasdec2;
			rgbSel = GREEN;
		}

		x = wasdec2.x;
		y = wasdec2.y;
		z = wasdec2.z;
		c = Capsule(glm::vec3(x, y, z), glm::vec3(x, y + 0.5f, z), 0.2f);

		DebugDraw::DrawCapsule(c.a, c.b, c.r, rgbSel);

		DebugDraw::DrawBox(gjk.p0, glm::vec3(0.05f, 0.05f, 0.05f), WHITE);
		DebugDraw::DrawBox(gjk.p1, glm::vec3(0.05f, 0.05f, 0.05f), PURPLE);
		DebugDraw::DrawLine(gjk.p0, gjk.p1, rgbSel);

		DebugDraw::Flush(ncamera);
	}
}
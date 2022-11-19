#pragma once

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
g3d::FreeCamera camera;
Transform transform;

Camera ncamera;

void InitTest()
{
	//SetMouseLock(true);

	ncamera.Teleport(0, 0, 0);
	ncamera.Enable();
	ncamera.m_speed = 3;

	camera.SetPosition(glm::vec3(10, 10, 10));



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
		model.Create("../data/models/crate.obj");
		model.SetMaterial(material);
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	camera.SimpleMove(deltaTime);
	camera.Update();
	//std::string ss = std::to_string(camera.GetPosition().x) + "|" + std::to_string(camera.GetPosition().y) + "|" + std::to_string(camera.GetPosition().z);
	//puts(ss.c_str());

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

	ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
	ncamera.Fps(mouse.x, mouse.y);

	std::string ss = std::to_string(ncamera.m_position.x) + "|" + std::to_string(ncamera.m_position.y) + "|" + std::to_string(ncamera.m_position.z);
	puts(ss.c_str());

	shader.Bind();

	//shader.SetUniform(viewUniform, camera.GetViewMatrix());
	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());
	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();
}
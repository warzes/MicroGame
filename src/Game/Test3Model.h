#pragma once

#include "6_Platform.h"
#include "8_oRenderer.h"

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 MVP;

out vec2 vTexCoord;

void main()
{
	gl_Position = MVP * vec4(vPos, 1.0);
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
Texture2D texture2d;
UniformLocation mvpUniform;
g3d::Model model;
g3d::Camera camera;

void InitTest()
{
	SetMouseLock(true);

	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		mvpUniform = shader.GetUniformVariable("MVP");
	}

	// Load Texture
	{
		Texture2DLoaderInfo info;
		info.fileName = "../data/textures/crate.png";
		texture2d.CreateFromFiles(info);
	}

	// Load geometry
	{
		model.Create("../data/models/crate.obj");
	}
}

void CloseTest()
{
	texture2d.Destroy();
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	camera.SimpleMove(deltaTime);
	camera.Update();

	texture2d.Bind(0);
	shader.Bind();
	glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix();
	shader.SetUniform(mvpUniform, MVP);
	model.Draw();
}
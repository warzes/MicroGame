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
Texture2D texture2d;
UniformLocation worldUniform;
UniformLocation viewUniform;
UniformLocation projectionUniform;
g3d::Model model;
g3d::FreeCamera camera;
Transform transform;

void InitTest()
{
	SetMouseLock(true);

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
	shader.SetUniform(worldUniform, transform.GetWorld());
	shader.SetUniform(viewUniform, camera.GetViewMatrix());
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	model.Draw();
}
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
g3d::Model customModel;
g3d::Material material;
g3d::FreeCamera camera;
Transform transform;
Transform transform2;

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
		info.verticallyFlip = true;
		material.diffuseTexture = TextureLoader::LoadTexture2D(info);
	}

	// Load geometry
	{
		model.Create("../data/models/crate.obj");
		model.SetMaterial(material);
	}

	// create custom model
	{
		std::vector<g3d::MeshCreateInfo> meshCreateInfo;
		meshCreateInfo.resize(1);

		meshCreateInfo[0].vertices = {
			{ {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f} },
			{ { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f} },
			{ { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
			{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
		};
		meshCreateInfo[0].indices = { 0, 1, 2, 2, 3, 0};

		meshCreateInfo[0].material = material;

		customModel.Create(std::move(meshCreateInfo));

		transform2.Translate({ 1.0f, 0.0f, 0.0f });
	}
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	camera.SimpleMove(deltaTime);
	camera.Update();

	std::string ss =  std::to_string(camera.GetPosition().x) + "|" + std::to_string(camera.GetPosition().y) + "|" + std::to_string(camera.GetPosition().z);
	puts(ss.c_str());

	shader.Bind();

	shader.SetUniform(viewUniform, camera.GetViewMatrix());
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();

	shader.SetUniform(worldUniform, transform2.GetWorld());
	customModel.Draw();
}
#pragma once

#include "6_Platform.h"
#include "8_oRenderer.h"

constexpr Vertex_Pos2_Color vertices[3] =
{
	{ {-0.6f, -0.4f}, {1.f, 0.f, 0.f} },
	{ { 0.6f, -0.4f}, {0.f, 1.f, 0.f} },
	{ {  0.f,  0.6f}, {0.f, 0.f, 1.f} }
};

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 MVP;

out vec3 color;

void main()
{
	gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	color = vCol;
}
)";

constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
)";

VertexArrayBuffer vao;
VertexBuffer vb;
ShaderProgram shader;
UniformVariable mvpUniform;

void InitTest()
{
	vb.Create(RenderResourceUsage::Static, 3, sizeof(vertices[0]), vertices);
	vao.Create<Vertex_Pos2_Color>(&vb, nullptr);

	shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
	mvpUniform = shader.GetUniformVariable("MVP");
}

void CloseTest()
{
	vb.Destroy();
	vao.Destroy();
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	static float dt = 0.0f;
	dt += deltaTime;
	float ratio = GetFrameBufferAspectRatio();
	
	glm::mat4 mvp = glm::mat4(1.0f);
	mvp = glm::rotate(mvp, dt, glm::vec3(0.0f, 0.0f, 1.0f));
	mvp = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f) * mvp;
	
	shader.Bind();
	shader.SetUniform(mvpUniform, mvp);
	vao.Draw();
}
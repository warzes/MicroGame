#pragma once

constexpr Vertex_Pos2_TexCoord vertices[] =
{
	{ {-0.9f, -0.9f}, {0.f, 0.f} },
	{ { 0.9f, -0.9f}, {1.f, 0.f} },
	{ {-0.9f,  0.9f}, {0.f, 1.f} },
	{ { 0.9f,  0.9f}, {1.f, 1.f} },
};
constexpr uint16_t indexData[] =
{
	0, 1, 2,
	1, 3, 2,
};

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	gl_Position = vec4(vPos, 0.0, 1.0);
	vTexCoord = aTexCoord;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

uniform sampler2D uSampler;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	vec4 textureClr = texture(uSampler, vTexCoord);
	if (textureClr.a < 0.02) discard;
	fragColor = textureClr;
}
)";

VertexArrayBuffer vao;
VertexBuffer vb;
IndexBuffer ib;
ShaderProgram shader;
Texture2D texture2d;

void InitTest()
{
	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
	}

	// Load Texture
	{
		texture2d.Create("../data/textures/crate.png");
	}

	// Load geometry
	{
		vb.Create(RenderResourceUsage::Static, 4, sizeof(vertices[0]), vertices);
		ib.Create(RenderResourceUsage::Static, 6, sizeof(uint16_t), indexData);
		vao.Create(&vb, &ib, &shader);
	}
}

void CloseTest()
{
	vb.Destroy();
	vao.Destroy();
	texture2d.Destroy();
	shader.Destroy();
}

void FrameTest(float deltaTime)
{
	texture2d.Bind(0);
	shader.Bind();
	vao.Draw();
}
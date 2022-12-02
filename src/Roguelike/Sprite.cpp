#include "stdafx.h"
#include "Sprite.h"

namespace
{
	constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWVP;
uniform vec2 uPos;
uniform vec2 uSize;
uniform vec3 uColor;

out vec2 vTexCoord;
out vec3 vColor;

void main()
{
	gl_Position = uWVP * uWorld * vec4(uPos.x + vPos.x * uSize.x, uPos.y + vPos.y * uSize.y, 0.0, 1.0);
	vTexCoord = aTexCoord;
	vColor = uColor;
}
)";
	constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;
in vec3 vColor;

uniform sampler2D uSampler;

out vec4 fragColor;

void main()
{
	vec4 textureClr = texture(uSampler, vTexCoord);
	fragColor = textureClr;
}
)";


	ShaderProgram shader;
	UniformLocation worldUniform;
	UniformLocation viewUniform;
	UniformLocation projectionUniform;
	UniformLocation colorUniform;
	UniformLocation sizeUniform;
https://www.markrjohnsongames.com/games/ultima-ratio-regum/

	Texture2D texture12x12;
	Texture2D texture12x12ru;

	VertexArrayBuffer vao;
	VertexBuffer vb;
	IndexBuffer ib;
}

void SpriteChar::Draw(float left, float top, float width, float height)
{
	if (!shader.IsValid())
	{
		// Load shader
		{
			shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
			shader.Bind();
			shader.SetUniform("uSampler", 0);
			worldUniform = shader.GetUniformVariable("uWVP");
			colorUniform = shader.GetUniformVariable("uColor");
			sizeUniform = shader.GetUniformVariable("uSize");
		}

		// Load texture
		{
			texture12x12.Create("../data/textures/12x12.png");
		}

		// Load Geometry
		{
			std::vector<Vertex_Pos2_TexCoord> vertices;
			std::vector<uint32_t> indices;
		}


		// Load geometry
		{
			constexpr Vertex_Pos2_TexCoord vertices[] =
			{
				{ {-0.5f, -0.5f}, {0.f, 0.f} },
				{ { 0.5f, -0.5f}, {1.f, 0.f} },
				{ {-0.5f,  0.5f}, {0.f, 1.f} },
				{ { 0.5f,  0.5f}, {1.f, 1.f} },
			};
			constexpr uint16_t indexData[] =
			{
				0, 1, 2,
				1, 3, 2,
			};

			vb.Create(RenderResourceUsage::Static, 4, sizeof(vertices[0]), vertices);
			ib.Create(RenderResourceUsage::Static, 6, sizeof(uint16_t), indexData);
			vao.Create(&vb, &ib, &shader);
		}
	}


	shader.Bind();

	shader.SetUniform(colorUniform, glm::vec3(1.0f, 1.0f, 1.0f));
	shader.SetUniform(sizeUniform, glm::vec2(32.0f, 32.0f));

	const float widthHeight = 480.0f;
	const float widthScreen = 480.0f * GetFrameBufferAspectRatio();
	glm::mat4 ortho = glm::ortho(0.0f, widthScreen, widthHeight, 0.0f);
	shader.SetUniform(worldUniform, ortho);
}
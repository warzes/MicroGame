#include "stdafx.h"
#include "Sprite.h"

namespace
{
	constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 atColor;

uniform mat4 uWVP;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
	gl_Position = uWVP * vec4(vPos, 0.0, 1.0);
	vTexCoord = aTexCoord;
	vColor = atColor;
}
)";
	constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uSampler;

out vec4 fragColor;

void main()
{
	vec4 texClr = texture(uSampler, vTexCoord);
	if (texClr.r < 0.01 && texClr.g < 0.01 && texClr.b < 0.01) discard;
	fragColor = texClr * vColor;
}
)";

	ShaderProgram shader;
	UniformLocation wvpUniform;

	Texture2D texture12x12;
	Texture2D texture12x12ru;

	VertexArrayBuffer vao;
	VertexBuffer vb;
	IndexBuffer ib;
}

void SpriteChar::Draw(const glm::vec2& pos, const glm::vec2& num, const glm::vec4& color)
{
	if (!shader.IsValid())
	{
		// Load shader
		{
			shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
			shader.Bind();
			shader.SetUniform("uSampler", 0);
			wvpUniform = shader.GetUniformVariable("uWVP");
		}

		// Load texture
		{
			texture12x12.Create("../data/textures/12x12.png");
		}

		// Load geometry
		{
			constexpr Vertex_Pos2_TexCoord_Color4 vertices[] =
			{
				{ {-0.5f, -0.5f}, {0.f, 0.f}, {1.0f, 1.0f, 1.0f, 1.0f} },
				{ { 0.5f, -0.5f}, {1.f, 0.f}, {1.0f, 1.0f, 1.0f, 1.0f} },
				{ {-0.5f,  0.5f}, {0.f, 1.f}, {1.0f, 1.0f, 1.0f, 1.0f} },
				{ { 0.5f,  0.5f}, {1.f, 1.f}, {1.0f, 1.0f, 1.0f, 1.0f} },
			};
			constexpr uint16_t indexData[] =
			{
				0, 1, 2,
				1, 3, 2,
			};

			vb.Create(RenderResourceUsage::Dynamic, 4, sizeof(vertices[0]), vertices);
			ib.Create(RenderResourceUsage::Static, 6, sizeof(uint16_t), indexData);
			vao.Create(&vb, &ib, &shader);
		}
	}

	texture12x12.Bind();

	shader.Bind();

	{
		const float numTileX = texture12x12.GetWidth() / 12.0f;
		const float numTileY = texture12x12.GetHeight() / 12.0f;

		const float tex1 = 1.0f / numTileX;
		const float tex2 = 1.0f / numTileY;
		const float t1 = 0.0f + tex1 * num.x;
		const float t2 = tex1 + tex1 * num.x;
		const float t3 = 0.0f + tex2 * (numTileY - num.y);
		const float t4 = tex2 + tex2 * (numTileY - num.y);

		const float sizeX = 12.0f;
		const float sizeY = 12.0f;

		const float posX = pos.x;
		const float posY = pos.y;

		const Vertex_Pos2_TexCoord_Color4 vertices[] =
		{
			{ {-0.5f*sizeX + posX, -0.5f*sizeX + posY}, {t1, t4}, {color.x, color.y, color.z, color.w} },
			{ { 0.5f*sizeX + posX, -0.5f*sizeX + posY}, {t2, t4}, {color.x, color.y, color.z, color.w} },
			{ {-0.5f*sizeX + posX,  0.5f*sizeX + posY}, {t1, t3}, {color.x, color.y, color.z, color.w} },
			{ { 0.5f*sizeX + posX,  0.5f*sizeX + posY}, {t2, t3}, {color.x, color.y, color.z, color.w} },
		};
		auto vb = vao.GetVertexBuffer();
		vb->Update(0, sizeof(vertices), vertices);
	}

	const float widthHeight = 480.0f;
	const float widthScreen = widthHeight * GetFrameBufferAspectRatio();
	glm::mat4 ortho = glm::ortho(0.0f, widthScreen, widthHeight, 0.0f, 0.0f, 1.0f);
	shader.SetUniform(wvpUniform, ortho);

	vao.Draw();
}
#include "stdafx.h"
#include "Sprite.h"
#include "DrawHelper.h"
//-----------------------------------------------------------------------------
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

	std::vector<Vertex_Pos2_TexCoord_Color4> vertex;
	std::vector<uint16_t> index;
	unsigned currentNumVertex = 0;
	unsigned currentNumIndex = 0;
	unsigned currentIndex = 0;
}
//-----------------------------------------------------------------------------
void SpriteChar::Init()
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
			vb.Create(RenderResourceUsage::Dynamic, 1, 1, nullptr);
			ib.Create(RenderResourceUsage::Dynamic, 1, 1, nullptr);
			vao.Create(&vb, &ib, &shader);
		}
	}
}
void SpriteChar::Close()
{
	vao.Destroy();
	vb.Destroy();
	ib.Destroy();
	texture12x12.Destroy();
	shader.Destroy();
}

void SpriteChar::Draw(const glm::vec2& pos, const glm::vec2& num, const glm::vec4& color)
{
	const float numTileX = texture12x12.GetWidth() / TileSize;
	const float numTileY = texture12x12.GetHeight() / TileSize;

	const float tex1 = 1.0f / numTileX;
	const float tex2 = 1.0f / numTileY;
	const float t1 = 0.0f + tex1 * num.x;
	const float t2 = tex1 + tex1 * num.x;
	const float t3 = 0.0f + tex2 * (numTileY - num.y);
	const float t4 = tex2 + tex2 * (numTileY - num.y);

	const float sizeX = TileSize;
	const float sizeY = TileSize;

	const float posX = pos.x * TileSize;
	const float posY = pos.y * TileSize;

	if (currentNumVertex + 3 >= vertex.size())
		vertex.resize(currentNumVertex + 4);

	vertex[currentNumVertex + 0] = { {-0.5f * sizeX + posX, -0.5f * sizeX + posY}, {t1, t4}, {color.x, color.y, color.z, color.w} };
	vertex[currentNumVertex + 1] = { { 0.5f * sizeX + posX, -0.5f * sizeX + posY}, {t2, t4}, {color.x, color.y, color.z, color.w} };
	vertex[currentNumVertex + 2] = { {-0.5f * sizeX + posX,  0.5f * sizeX + posY}, {t1, t3}, {color.x, color.y, color.z, color.w} };
	vertex[currentNumVertex + 3] = { { 0.5f * sizeX + posX,  0.5f * sizeX + posY}, {t2, t3}, {color.x, color.y, color.z, color.w} };

	currentNumVertex = currentNumVertex + 4;

	if (currentNumIndex + 5 >= index.size())
		index.resize(currentNumIndex + 6);

	index[currentNumIndex + 0] = currentIndex + 0;
	index[currentNumIndex + 1] = currentIndex + 1;
	index[currentNumIndex + 2] = currentIndex + 2;
	index[currentNumIndex + 3] = currentIndex + 1;
	index[currentNumIndex + 4] = currentIndex + 3;
	index[currentNumIndex + 5] = currentIndex + 2;

	currentNumIndex = currentNumIndex + 6;
	currentIndex = currentIndex + 4;
}

void SpriteChar::DrawInMapScreen(const glm::vec2& pos, const glm::vec2& num, const glm::vec4& color)
{
	int leftMapScreen = 1;
	int rightMapScreen = 1;
	int topMapScreen = 1;
	int bottomMapScreen = 1;
	DrawHelper::GetScreenWorldViewport(leftMapScreen, rightMapScreen, topMapScreen, bottomMapScreen);

	if (pos.x <= leftMapScreen || pos.x >= rightMapScreen ||
		pos.y <= topMapScreen || pos.y >= bottomMapScreen)
		return;

	Draw(pos, num, color);
}

void SpriteChar::Flush()
{
	texture12x12.Bind();
	shader.Bind();
	const float widthHeight = ScreenHeight;
	const float widthScreen = widthHeight * GetFrameBufferAspectRatio();
	glm::mat4 ortho = glm::ortho(0.0f, widthScreen, widthHeight, 0.0f, 0.0f, 1.0f);
	shader.SetUniform(wvpUniform, ortho);

	auto pvb = vao.GetVertexBuffer();
	pvb->Update(0, currentNumVertex, sizeof(Vertex_Pos2_TexCoord_Color4), vertex.data());

	auto pib = vao.GetIndexBuffer();
	pib->Update(0, currentNumIndex, sizeof(uint16_t), index.data());

	vao.Draw();

	currentNumVertex = 0;
	currentNumIndex = 0;
	currentIndex = 0;
}
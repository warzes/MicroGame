#include "stdafx.h"
#include "MinimapRender.h"
#include "World.h"
#include "Character.h"
#include "DrawHelper.h"
/*
TODO: сейчас я рендерю "попиксельно" что неэффективно так как каждый пиксель рендерится квадом. 
отрисовка Point не подошла так как размер пикселя надо менять с учетом размера экрана
Лучшим решением будет сохранить набор комнат и коридоров и рисовать всю комнату или коридор одним квадому
*/
//-----------------------------------------------------------------------------
namespace
{
	static constexpr const char* MinimapVertexShader = R"(
#version 330 core

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec3 vertexColor;
uniform mat4 MVP;
out vec3 out_color;
void main()
{
	gl_Position =  MVP * vec4(vertexPosition, 0.0, 1.0);
	out_color = vertexColor;
}
)";

	static constexpr const char* MinimapFragmentShader = R"(
#version 330 core
in vec3 out_color;
out vec4 fragcolor;
void main()
{
	fragcolor = vec4(out_color, 1.0);
}
)";	
}
//-----------------------------------------------------------------------------
bool MinimapRender::Create()
{	
	m_shaderProgramQuad.CreateFromMemories(MinimapVertexShader, MinimapFragmentShader);
	m_ortho = m_shaderProgramQuad.GetUniformVariable("MVP");

	m_vertexBufQuad.Create(RenderResourceUsage::Dynamic, 1, 1, nullptr);
	m_indexBufQuad.Create(RenderResourceUsage::Dynamic, 1, 1, nullptr);
	m_vaoQuad.Create(&m_vertexBufQuad, &m_indexBufQuad, &m_shaderProgramQuad);

	return true;
}
//-----------------------------------------------------------------------------
void MinimapRender::Destroy()
{
	m_vaoQuad.Destroy();
	m_indexBufQuad.Destroy();
	m_vertexBufQuad.Destroy();
	m_shaderProgramQuad.Destroy();
}
//-----------------------------------------------------------------------------
void MinimapRender::Draw(const Map& map, const Player& player)
{
	glDisable(GL_DEPTH_TEST);

	int left;
	int right;
	int top;
	int bottom;
	DrawHelper::GetScreenMiniMapViewport(left, right, top, bottom);
	right--;
	bottom--;

	const float sizeX = (right - left)*TileSize / (float)SizeMap;
	const float sizeY = (bottom - top)*TileSize / (float)SizeMap;

	glm::vec3 color = glm::vec3(1.0f);
	for (size_t x = 0; x < SizeMap; x++)
	{
		for (size_t y = 0; y < SizeMap; y++)
		{
			if (map.tiles[x][y].IsFloor())
			{
				color = glm::vec3(0.0f, 1.0f, 1.0f);
				
			}
			else if (map.tiles[x][y].IsWall())
			{
				color = glm::vec3(1.0f, 0.0f, 1.0f);
			}
			else
				continue;

			// левая верхняя позиция в пикселях + позиция с карты в пикселях + половина размера тайла
			const float posX = left * TileSize + x * sizeX + TileSize / 2.0f;
			const float posY = top * TileSize + y * sizeY + TileSize / 2.0f;

			addQuad(posX, posY, sizeX, sizeY, 0.0f, 0.0f, color);
		}
	}

	// add player
	{
		const float posX = left * TileSize + player.x * sizeX + TileSize / 2.0f;
		const float posY = top * TileSize + player.y * sizeY + TileSize / 2.0f;
		// чтобы четче видеть игрока
		const float offsetX = sizeX / 2.0f;
		const float offsetY = sizeY / 2.0f;

		addQuad(posX, posY, sizeX, sizeY, offsetX, offsetY, glm::vec3(0.0f, 0.2f, 1.0f));
	}

	m_shaderProgramQuad.Bind();
	m_shaderProgramQuad.SetUniform(m_ortho, DrawHelper::GetOrtho());

	VertexArrayBuffer::UnBind();

	auto pvb = m_vaoQuad.GetVertexBuffer();
	pvb->Update(0, currentNumVertex, sizeof(Vertex_Pos2_Color), vertex.data());

	auto pib = m_vaoQuad.GetIndexBuffer();
	pib->Update(0, currentNumIndex, sizeof(uint16_t), index.data());

	m_vaoQuad.Draw();

	currentNumVertex = 0;
	currentNumIndex = 0;
	currentIndex = 0;

	glEnable(GL_DEPTH_TEST);
}
//-----------------------------------------------------------------------------
void MinimapRender::addQuad(float posX, float posY, float sizeX, float sizeY, float offsetX, float offsetY, const glm::vec3& color)
{
	if (currentNumVertex + 3 >= vertex.size())
		vertex.resize(currentNumVertex + 4);

	vertex[currentNumVertex + 0] = { {0.0f  + posX - offsetX, 0.0f  + posY - offsetY}, color };
	vertex[currentNumVertex + 1] = { {sizeX + posX + offsetX, 0.0f  + posY - offsetY}, color };
	vertex[currentNumVertex + 2] = { {0.0f  + posX - offsetX, sizeX + posY + offsetY}, color };
	vertex[currentNumVertex + 3] = { {sizeX + posX + offsetX, sizeX + posY + offsetY}, color };

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
//-----------------------------------------------------------------------------
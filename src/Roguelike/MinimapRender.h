#pragma once

class World;

class MinimapRender
{
public:
	bool Create();
	void Destroy();

	void Draw(const World& world);

private:
	void addQuad(float posX, float posY, float sizeX, float sizeY, float offsetX, float offsetY, const glm::vec3& color);

	VertexArrayBuffer m_vaoQuad;
	VertexBuffer m_vertexBufQuad;
	IndexBuffer m_indexBufQuad;
	ShaderProgram m_shaderProgramQuad;
	UniformLocation m_ortho;

	std::vector<Vertex_Pos2_Color> vertex;
	std::vector<uint16_t> index;
	unsigned currentNumVertex = 0;
	unsigned currentNumIndex = 0;
	unsigned currentIndex = 0;

	int m_width = 0;
	int m_height = 0;
};
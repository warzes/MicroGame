#include "stdafx.h"
#include "TestGenMap.h"

constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uColor;

out vec2 vTexCoord;
out vec3 vColor;

void main()
{
	gl_Position = uProjection * uView * uWorld * vec4(vPos, 1.0);
	vTexCoord = aTexCoord;
	vColor = uColor;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec2 vTexCoord;
in vec3 vColor;

out vec4 fragColor;

void main()
{
	//vec4 textureClr = texture(uSampler, vTexCoord);
	//if (textureClr.a < 0.02) discard;
	fragColor = vec4(vColor.r, vColor.g, vColor.b, 1.0);
	fragColor = vec4(vTexCoord.x, vTexCoord.y, 1.0-vTexCoord.y, 1.0);
}
)";

ShaderProgram testShader;
UniformLocation testWorldUniform;
UniformLocation testViewUniform;
UniformLocation testProjectionUniform;
UniformLocation testColorUniform;


Transform testTransform;

enum class TileHeight
{
	None,
	Stage,
	SlopeLeft,
	SlopeRight,
	SlopeForward,
	SlopeBack,

};

class PolyMap
{
public:
	PolyMap()
	{
		m_meshCreateInfo.resize(1);
	}
	void Clear()
	{
		m_currentIndex = 0;
		m_meshCreateInfo.clear();
		m_model.Destroy();
		m_meshCreateInfo.resize(1);
	}

	void AddQuad(const glm::vec3& pos, TileHeight height = TileHeight::None,
		const glm::vec2& texPos0 = {0.0f, 0.0f}, const glm::vec2& texPos1 = { 1.0f, 1.0f })
	{
		m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f + pos.y,  0.5f + pos.z}, {texPos0.x, texPos1.y} });
		m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, 0.0f + pos.y,  0.5f + pos.z}, {texPos1.x, texPos1.y} });
		m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, 0.0f + pos.y, -0.5f + pos.z}, {texPos1.x, texPos0.y} });
		m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f + pos.y, -0.5f + pos.z}, {texPos0.x, texPos0.y} });
		AddIndex();

		if (height == TileHeight::Stage)
		{
			// left
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f,   0.5f + pos.z}, {texPos0.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, pos.y,  0.5f + pos.z}, {texPos1.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, pos.y, -0.5f + pos.z}, {texPos1.x, texPos0.y} });
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f,  -0.5f + pos.z}, {texPos0.x, texPos0.y} });
			AddIndex();

			// Right
			m_meshCreateInfo[0].vertices.push_back({ {0.5f + pos.x, 0.0f,   0.5f + pos.z}, {texPos0.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {0.5f + pos.x, pos.y,  0.5f + pos.z}, {texPos1.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {0.5f + pos.x, pos.y, -0.5f + pos.z}, {texPos1.x, texPos0.y} });
			m_meshCreateInfo[0].vertices.push_back({ {0.5f + pos.x, 0.0f,  -0.5f + pos.z}, {texPos0.x, texPos0.y} });
			AddIndex();

			// Forward
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f,  0.5f + pos.z}, {texPos0.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, pos.y, 0.5f + pos.z}, {texPos1.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, pos.y, 0.5f + pos.z}, {texPos1.x, texPos0.y} });
			m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, 0.0f,  0.5f + pos.z}, {texPos0.x, texPos0.y} });
			AddIndex();

			// Back
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, 0.0f,  -0.5f + pos.z}, {texPos0.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ {-0.5f + pos.x, pos.y, -0.5f + pos.z}, {texPos1.x, texPos1.y} });
			m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, pos.y, -0.5f + pos.z}, {texPos1.x, texPos0.y} });
			m_meshCreateInfo[0].vertices.push_back({ { 0.5f + pos.x, 0.0f,  -0.5f + pos.z}, {texPos0.x, texPos0.y} });
			AddIndex();
		}
	}

	void AddIndex()
	{
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 0);
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 1);
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 2);
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 2);
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 3);
		m_meshCreateInfo[0].indices.push_back(m_currentIndex + 0);
		m_currentIndex += 4;
	}

	void Finish()
	{
		// TODO: выбросить лишние полигоны

		m_model.Create(std::move(m_meshCreateInfo));
	}

	void Draw()
	{
		m_model.Draw();
	}

private:
	std::vector<g3d::MeshCreateInfo> m_meshCreateInfo;
	unsigned m_currentIndex;
	g3d::Model m_model;
};

PolyMap map;

void TestGenMap::Init()
{
	// Load shader
	{
		testShader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		testShader.Bind();
		testWorldUniform = testShader.GetUniformVariable("uWorld");
		testViewUniform = testShader.GetUniformVariable("uView");
		testProjectionUniform = testShader.GetUniformVariable("uProjection");
		testColorUniform = testShader.GetUniformVariable("uColor");
	}

	// create custom model
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		//map.AddQuad({ 0.0f, 0.0f, 0.0f });
		//map.AddQuad({ 1.0f, 0.0f, 0.0f });
		//map.AddQuad({ 0.0f, 0.0f, 1.0f });
		//map.AddQuad({ 1.0f, 0.0f, 1.0f });

		//map.AddQuad({ 2.0f, 0.5f, 1.0f }, TileHeight::Stage);
		//map.AddQuad({ 2.0f, -0.5f, 2.0f }, TileHeight::Stage);

		NewGen();


		map.Finish();
		auto t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> ms_double = t2 - t1;

		LogPrint("VertexMap Gen = " + std::to_string(ms_double.count()) + " s.");

		testTransform.Translate({ 0.0f, 0.0f, 0.0f });
	}
}

void TestGenMap::Destroy()
{
	testShader.Destroy();
	map.Clear();
}

struct CircleTile
{
	glm::vec3 pos;
	float radius;
};

std::vector<CircleTile> circles;
CircleTile centr;

void TestGenMap::NewGen()
{
	circles.clear();

	centr.radius = rand() % 10+5;
	centr.pos = glm::vec3(0.0f);

	circles.push_back(centr);

	for (int i = 0; i < 100; i++)
	{
		CircleTile c;
		c.radius = rand() % 10+5;
		c.pos.x = (rand() % 50 - 25) ;
		c.pos.z = (rand() % 50 - 25);
		circles.push_back(c);
	}

	for (int i = 0; i < 100; i++)
	{
		for (int x = -circles[i].radius; x < circles[i].radius; x++)
		{
			for (int y = -circles[i].radius; y < circles[i].radius; y++)
			{
				glm::vec3 pos1 = circles[i].pos;
				pos1.x += x;
				pos1.z += y;

				float distance = floor(glm::distance(pos1, circles[i].pos) +0.5f);

				if (distance < circles[i].radius)
				{
					map.AddQuad({pos1.x, 0.0f, pos1.z });
				}
			}
		}
	}
}

void TestGenMap::Draw(const Camera& camera)
{
	testShader.Bind();

	testShader.SetUniform(testViewUniform, camera.m_view);
	testShader.SetUniform(testProjectionUniform, GetCurrentProjectionMatrix());

	testShader.SetUniform(testColorUniform, glm::vec3(1.0f,1.0f,1.0f));
	testShader.SetUniform(testWorldUniform, testTransform.GetWorld());
	map.Draw();
}
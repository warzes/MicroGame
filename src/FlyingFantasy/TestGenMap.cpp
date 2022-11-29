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
	fragColor = vec4(1.0, 0.8, 1.0, 1.0);
}
)";

ShaderProgram testShader;
UniformLocation testWorldUniform;
UniformLocation testViewUniform;
UniformLocation testProjectionUniform;
UniformLocation testColorUniform;

g3d::Model testModel;
Transform testTransform;

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
}

void TestGenMap::Destroy()
{

}

void TestGenMap::NewGen()
{

}

void TestGenMap::Draw(const Camera& camera)
{
}
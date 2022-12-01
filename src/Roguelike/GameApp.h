#pragma once

//-----------------------------------------------------------------------------

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
in float posY;

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
UniformLocation colorUniform;

g3d::Material material;

bool StartGameApp()
{
	// Load shader
	{
		shader.CreateFromMemories(vertex_shader_text, fragment_shader_text);
		shader.Bind();
		shader.SetUniform("uSampler", 0);
		worldUniform = shader.GetUniformVariable("uWorld");
		viewUniform = shader.GetUniformVariable("uView");
		projectionUniform = shader.GetUniformVariable("uProjection");
		colorUniform = shader.GetUniformVariable("uColor");
	}

	// Load Texture
	{
		material.diffuseTexture = TextureLoader::LoadTexture2D("../data/textures/tileset.png");
	}

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	return true;
}

void CloseGameApp()
{
	shader.Destroy();
}

void UpdateGameApp(float deltaTime)
{
	
}

void FrameGameApp(float deltaTime)
{
}
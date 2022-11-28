#pragma once

//-----------------------------------------------------------------------------
// Concept
// https://www.youtube.com/watch?v=yLQLoVcowLU
// https://www.youtube.com/watch?v=0Id0LQj7pIs
// https://github.com/jdah/microcraft
// Halloween3D
// claudette

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

g3d::Model model;
g3d::Material material;
Transform transform;

Camera ncamera;


bool StartGameApp()
{
	// Init Camera
	{
		ncamera.Teleport(0, 3, -6);
		ncamera.LookAt(glm::vec3(0, 0, 0));
		ncamera.Enable();
		ncamera.m_speed = 5;
	}

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

	// Load geometry
	{
		model.Create("../data/models/map.obj");
		model.SetMaterial(material);
		transform.Translate(0, 0, 0);
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
	// camera
	{
		bool active = IsMouseButtonDown(0);
		SetMouseLock(active);

		const float xpos = GetMouseX();
		const float ypos = GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		glm::vec2 mouse = tempMath::scale2(glm::vec2((lastPosX - xpos), (lastPosY - ypos)), 200.0f * deltaTime * active);
		lastPosX = xpos;
		lastPosY = ypos;

		glm::vec3 wasdec = tempMath::scale3(glm::vec3(IsKeyboardKeyDown(KEY_A) - IsKeyboardKeyDown(KEY_D), IsKeyboardKeyDown(KEY_E) - IsKeyboardKeyDown(KEY_C), IsKeyboardKeyDown(KEY_W) - IsKeyboardKeyDown(KEY_S)), ncamera.m_speed * deltaTime);

		ncamera.Move(wasdec.x, wasdec.y, wasdec.z);
		ncamera.Fps(mouse.x, mouse.y);
	}
}

void FrameGameApp(float deltaTime)
{
	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());

	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();



	Triangle tri(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(2.0f, 0.0f, 2.0f));


	//DebugDraw::DrawCapsule(
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 1.0f, 0.0f),
	//	0.5f, RED);

	DebugDraw::DrawTriangle(tri.verts[0], tri.verts[1], tri.verts[2], RED);
	DebugDraw::DrawBox(tri.center, glm::vec3(0.1f), BLUE);


	DebugDraw::Flush(ncamera);
}
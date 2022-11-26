#pragma once

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
Poly modelPoly;
g3d::Material material;
Transform transform;

Camera ncamera;

VertexArrayBuffer vao;
VertexBuffer vb;

void InitTest()
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

		struct  tt
		{
			static void AddQuad(std::vector<Vertex_Pos3_TexCoord>& vertices, glm::vec3 pos)
			{
				std::vector<Vertex_Pos3_TexCoord> temp =
				{
					{ {-1.0f + pos.x, 0.0f + pos.y - 2.0f, -1.0f + pos.z}, {0.f, 0.f} },
					{ {-1.0f + pos.x, 0.0f + pos.y - 2.0f,  1.0f + pos.z}, {0.f, 1.f} },
					{ { 1.0f + pos.x, 0.0f + pos.y - 2.0f, -1.0f + pos.z}, {1.f, 1.f} },
					{ {-1.0f + pos.x, 0.0f + pos.y - 2.0f,  1.0f + pos.z}, {0.f, 1.f} },
					{ { 1.0f + pos.x, 0.0f + pos.y - 2.0f,  1.0f + pos.z}, {1.f, 1.f} },
					{ { 1.0f + pos.x, 0.0f + pos.y - 2.0f, -1.0f + pos.z}, {1.f, 1.f} },
				};

				for (size_t i = 0; i < temp.size(); i++)
				{
					vertices.push_back(temp[i]);
				}
			}
		};

		std::vector<Vertex_Pos3_TexCoord> vertices;

		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				tt::AddQuad(vertices, glm::vec3((x - 5) * 2, 0.0f, (y - 5) * 2));
			}
		}

		vb.Create(RenderResourceUsage::Static, vertices.size(), sizeof(vertices[0]), vertices.data());
		vao.Create(&vb, nullptr, &shader);

		for (size_t i = 0; i < vertices.size(); i++)
		{
			modelPoly.verts.push_back(vertices[i].position);
		}
		modelPoly.cnt = vertices.size();

		//modelPoly = model.GetPoly();
	}

	//RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));
}

void CloseTest()
{
	shader.Destroy();
}

void FrameTest(float deltaTime)
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

	shader.Bind();

	shader.SetUniform(viewUniform, ncamera.m_view);
	shader.SetUniform(projectionUniform, GetCurrentProjectionMatrix());
	shader.SetUniform(colorUniform, glm::vec3(0, 0, 1));
	shader.SetUniform(worldUniform, transform.GetWorld());
	model.Draw();

	vao.Draw();

	DebugDraw::DrawGrid(0);

	DebugDraw::Flush(ncamera);
}
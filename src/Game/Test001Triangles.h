#pragma once

#define USE_TESTCOMMANDBUFFER 1
#define USE_LOADSHADERFROMMEMORY 1

constexpr Vertex_Pos2_Color vertices[] =
{
	{ {-0.6f, -0.4f}, {1.f, 0.f, 0.f} },
	{ { 0.6f, -0.4f}, {0.f, 1.f, 0.f} },
	{ {  0.f,  0.6f}, {0.f, 0.f, 1.f} }
};

#if USE_LOADSHADERFROMMEMORY
constexpr const char* vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 MVP;

out vec3 color;

void main()
{
	gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	color = vCol;
}
)";
constexpr const char* fragment_shader_text = R"(
#version 330 core

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
)";
#endif

VertexArrayBuffer vao;
VertexBuffer vb;
ShaderProgram* shader;
UniformLocation mvpUniform;

void InitTest()
{
	// Load shader
	{
#if USE_LOADSHADERFROMMEMORY
		shader = new ShaderProgram();
		shader->CreateFromMemories(vertex_shader_text, fragment_shader_text);
#else
		shader = ShaderLoader::Load("../data/shaders/Test1Triangles.glsl");
#endif
		mvpUniform = shader->GetUniformVariable("MVP");
	}

	// Load geometry
	{
		vb.Create(RenderResourceUsage::Static, 3, sizeof(vertices[0]), vertices);

		/*
		* Example variants:
		*	Example 1:
		*		vao.Create(&vb, nullptr, GetVertexAttributes<Vertex_Pos2_Color>());
		*	Example 2:
		*		vao.Create<Vertex_Pos2_Color>(&vb, nullptr);
		*	Example 3:
		*		std::vector<VertexAttributeRaw> attrs = {{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(Vertex_Pos3), .pointer = (void*)offsetof(Vertex_Pos3, position)}};
		*		vao.Create<Vertex_Pos2_Color>(&vb, nullptr, attrs);
		*/
		vao.Create(&vb, nullptr, shader);
	}	
}

void CloseTest()
{
	vb.Destroy();
	vao.Destroy();
	shader->Destroy();
	delete shader;
}

#if USE_TESTCOMMANDBUFFER
// TODO: в графику?
struct CommandBuffer
{
	VertexArrayBuffer* vao = nullptr;
	ShaderProgram* shader = nullptr;

	struct Uniforms
	{
		enum class UniformDataType
		{
			Int,
			Float,
			Mat4
		};

		union UniformData
		{
			int Int;
			float Float;
			glm::mat4 Mat4;
		};

		UniformData data;
		UniformDataType type;
	};

	std::vector<std::pair<UniformLocation, Uniforms>> uniforms;

	void SetUniform(UniformLocation var, Uniforms type)
	{
		bool isFind = false;
		for (size_t i = 0; i < uniforms.size(); i++)
		{
			if (uniforms[i].first == var) 
			{
				uniforms[i].second = type;
				isFind = true;
				break;
			}
		}
		if (isFind == false)
		{
			uniforms.emplace_back(std::make_pair(var, type));
		}
	}

	void SetUniform(UniformLocation var, const glm::mat4& mat)
	{
		Uniforms type;
		type.type = Uniforms::UniformDataType::Mat4;
		type.data.Mat4 = mat;
		SetUniform(var, type);
	}

	void Submit()
	{
		if (shader && shader->IsValid())
		{
			shader->Bind();
			for (size_t i = 0; i < uniforms.size(); i++)
			{
				Uniforms::UniformDataType currentDataType = uniforms[i].second.type;
				if (currentDataType == Uniforms::UniformDataType::Mat4)
					shader->SetUniform(uniforms[i].first, uniforms[i].second.data.Mat4);
			}
		}
		if (vao && vao->IsValid())
			vao->Draw();
	}
};
#endif

void FrameTest(float deltaTime)
{
	static float dt = 0.0f;
	dt += deltaTime;
	float ratio = GetFrameBufferAspectRatio();
	
	glm::mat4 mvp = glm::mat4(1.0f);
	mvp = glm::rotate(mvp, dt, glm::vec3(0.0f, 0.0f, 1.0f));
	mvp = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f) * mvp;

#if USE_TESTCOMMANDBUFFER
	CommandBuffer cb;
	cb.vao = &vao;
	cb.shader = shader;
	cb.SetUniform(mvpUniform, mvp);
	cb.Submit();
#else	
	shader->Bind();
	shader->SetUniform(mvpUniform, mvp);
	vao.Draw();
#endif
}
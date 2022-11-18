#include "stdafx.h"
#include "3_Core.h"
#include "6_Platform.h"
#include "8_Renderer.h"
#include "9_Graphics.h"

#include <stb/stb_truetype.h>
#include <tiny_obj_loader.h>

namespace std
{
	template <>
	struct hash<Vertex_Pos3_TexCoord>
	{
		size_t operator()(const Vertex_Pos3_TexCoord& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1);
		}
	};
} // namespace std


#pragma region Graphics3D
namespace g3d
{

	void FreeCamera::MoveForward(float deltaTime, float speedMod)
	{
		m_position += m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveBackward(float deltaTime, float speedMod)
	{
		m_position -= m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveRight(float deltaTime, float speedMod)
	{
		m_position -= m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveLeft(float deltaTime, float speedMod)
	{
		m_position += m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveUp(float deltaTime, float speedMod)
	{
		m_position += m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::MoveDown(float deltaTime, float speedMod)
	{
		m_position -= m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void FreeCamera::Rotate(float offsetX, float offsetY)
	{
		m_yaw -= offsetX * m_sensitivity;
		m_pitch += offsetY * m_sensitivity;
		SetRotate(m_yaw, m_pitch);
	}

	void FreeCamera::SetRotate(float yaw, float pitch)
	{
		m_yaw = yaw;
		m_pitch = pitch;
		if (m_pitch > 89.0f) m_pitch = 89.0f;
		else if (m_pitch < -89.0f) m_pitch = -89.0f;
		if (m_yaw > 360.0f) m_yaw = 0.0f;
		else if (m_yaw < -360.0f) m_yaw = 0.0f;
		updateVectors();
	}

	void FreeCamera::SimpleMove(float deltaTime)
	{
		const float xpos = GetMouseX();
		const float ypos = GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		Rotate((xpos - lastPosX), (lastPosY - ypos));
		lastPosX = xpos;
		lastPosY = ypos;

		constexpr float speedMod = 1.0f;
		if (IsKeyboardKeyDown(KEY_W))
			MoveForward(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_S))
			MoveBackward(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_D))
			MoveRight(deltaTime, speedMod);
		if (IsKeyboardKeyDown(KEY_A))
			MoveLeft(deltaTime, speedMod);

		constexpr float speedRotateMod = 1600.0f;
		if (IsKeyboardKeyDown(KEY_E))
			Rotate(speedRotateMod * deltaTime, 0.0f);
		if (IsKeyboardKeyDown(KEY_Q))
			Rotate(-speedRotateMod * deltaTime, 0.0f);

#ifdef _DEBUG
		if (IsKeyboardKeyDown(KEY_T))
			MoveUp(deltaTime, speedMod / 2.0f);
		if (IsKeyboardKeyDown(KEY_G))
			MoveDown(deltaTime, speedMod / 2.0f);

		if (IsKeyboardKeyDown(KEY_R))
			Rotate(0.0f, speedRotateMod * deltaTime);
		if (IsKeyboardKeyDown(KEY_F))
			Rotate(0.0f, -speedRotateMod * deltaTime);
#endif

		Update();
	}

	void FreeCamera::Update()
	{
		m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
	}

	//Frustum Camera::ComputeFrustum() const
	//{
	//	Frustum frustum;

	//	const float halfVSide = m_far * tanf(m_fov * 0.5f);
	//	const float halfHSide = halfVSide * GetWindowAspect();
	//	const glm::vec3 frontMultFar = m_far * m_front;

	//	frustum.nearFace = { m_position + m_near * m_front, m_front };
	//	frustum.farFace = { m_position + frontMultFar, -m_front };
	//	frustum.rightFace = { m_position, glm::cross(m_up, frontMultFar + m_right * halfHSide) };
	//	frustum.leftFace = { m_position, glm::cross(frontMultFar - m_right * halfHSide, m_up) };
	//	frustum.topFace = { m_position, glm::cross(m_right, frontMultFar - m_up * halfVSide) };
	//	frustum.bottomFace = { m_position, glm::cross(frontMultFar + m_up * halfVSide, m_right) };

	//	return frustum;
	//}

	void FreeCamera::updateVectors()
	{
		const float radiansYaw = glm::radians(m_yaw);
		const float radiansPitch = glm::radians(m_pitch);

		const glm::vec3 front = {
			cos(radiansYaw) * cos(radiansPitch),
			sin(radiansPitch),
			sin(radiansYaw) * cos(radiansPitch)
		};
		m_front = glm::normalize(front);
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

	bool Model::Create(const char* fileName, const char* pathMaterialFiles)
	{
		Destroy();

		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = pathMaterialFiles; // Path to material files

		tinyobj::ObjReader reader;
		if (!reader.ParseFromFile(fileName, readerConfig))
		{
			if (!reader.Error().empty())
				LogError("TinyObjReader: " + reader.Error());
			return false;
		}
		if (!reader.Warning().empty())
			LogWarning("TinyObjReader: " + reader.Warning());

		auto& attributes = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		const bool isFindMaterials = !materials.empty();

		std::vector<Mesh> tempMesh(materials.size());
		std::vector<std::unordered_map<Vertex_Pos3_TexCoord, uint32_t>> uniqueVertices(materials.size());
		if (tempMesh.empty())
		{
			tempMesh.resize(1);
			uniqueVertices.resize(1);
		}

		// Loop over shapes
		for (size_t shapeId = 0; shapeId < shapes.size(); shapeId++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t faceId = 0; faceId < shapes[shapeId].mesh.num_face_vertices.size(); faceId++)
			{
				const size_t fv = static_cast<size_t>(shapes[shapeId].mesh.num_face_vertices[faceId]);

				// per-face material
				int materialId = shapes[shapeId].mesh.material_ids[faceId];
				if (materialId < 0) materialId = 0;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					const tinyobj::index_t idx = shapes[shapeId].mesh.indices[index_offset + v];

					// vertex position
					const tinyobj::real_t vx = attributes.vertices[3 * size_t(idx.vertex_index) + 0];
					const tinyobj::real_t vy = attributes.vertices[3 * size_t(idx.vertex_index) + 1];
					const tinyobj::real_t vz = attributes.vertices[3 * size_t(idx.vertex_index) + 2];

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attributes.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attributes.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attributes.normals[3 * size_t(idx.normal_index) + 2];
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					tinyobj::real_t tx = 0;
					tinyobj::real_t ty = 0;
					if (idx.texcoord_index >= 0)
					{
						tx = attributes.texcoords[2 * size_t(idx.texcoord_index) + 0];
						ty = attributes.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					// vertex colors
					const tinyobj::real_t r = attributes.colors[3 * size_t(idx.vertex_index) + 0];
					const tinyobj::real_t g = attributes.colors[3 * size_t(idx.vertex_index) + 1];
					const tinyobj::real_t b = attributes.colors[3 * size_t(idx.vertex_index) + 2];

					glm::vec3 position{ vx, vy, vz };
					glm::vec2 texCoord{ tx,ty };
					Vertex_Pos3_TexCoord vertex{ position, texCoord };

					if (uniqueVertices[materialId].count(vertex) == 0)
					{
						uniqueVertices[materialId][vertex] = static_cast<uint32_t>(tempMesh[materialId].vertices.size());
						tempMesh[materialId].vertices.emplace_back(vertex);
					}

					tempMesh[materialId].indices.emplace_back(uniqueVertices[materialId][vertex]);
				}
				index_offset += fv;
			}
		}

		// load materials
		bool isFindToTransparent = false;
		if (isFindMaterials)
		{
			for (int i = 0; i < materials.size(); i++)
			{
				if (materials[i].diffuse_texname.empty()) continue;

				std::string diffuseMap = pathMaterialFiles + materials[i].diffuse_texname;
				tempMesh[i].material.diffuseTexture = TextureLoader::LoadTexture2D(diffuseMap.c_str());
				if (!isFindToTransparent && tempMesh[i].material.diffuseTexture)
					isFindToTransparent = tempMesh[i].material.diffuseTexture->isTransparent;
			}
		}

		// сортировка по прозрачности
		if (isFindToTransparent)
		{
			std::vector<Mesh> tempMesh2;

			// TODO: медленно, оптимизировать

			// сначала непрозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (!tempMesh[i].material.diffuseTexture)
					tempMesh2.push_back(tempMesh[i]);
				else if (!tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}
			// теперь прозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}

			m_subMeshes = std::move(tempMesh2);
		}
		else
			m_subMeshes = std::move(tempMesh);

		return createBuffer();
	}

	bool g3d::Model::Create(std::vector<MeshCreateInfo>&& meshes)
	{
		Destroy();
		m_subMeshes.resize(meshes.size());
		for (int i = 0; i < meshes.size(); i++)
			m_subMeshes[i].Set(std::move(meshes[i]));

		return createBuffer();
	}

	void Model::Destroy()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i].vertices.clear();
			m_subMeshes[i].indices.clear();

			m_subMeshes[i].vertexBuffer.Destroy();
			m_subMeshes[i].indexBuffer.Destroy();
			m_subMeshes[i].vao.Destroy();
		}
		m_subMeshes.clear();
	}

	void Model::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
				m_subMeshes[i].vao.SetInstancedBuffer(instanceBuffer, attribs);
		}
	}

	void Model::Draw(uint32_t instanceCount)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
			{
				const Texture2D* diffuseTexture = m_subMeshes[i].material.diffuseTexture;
				if (diffuseTexture && diffuseTexture->IsValid())
					diffuseTexture->Bind(0);
				m_subMeshes[i].vao.Draw(PrimitiveDraw::Triangles, instanceCount);
			}
		}
	}

	void Model::SetMaterial(const Material& material)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i].material = material;
		}
	}

	bool Model::createBuffer()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (!m_subMeshes[i].vertexBuffer.Create(RenderResourceUsage::Static, m_subMeshes[i].vertices.size(), sizeof(m_subMeshes[i].vertices[0]), m_subMeshes[i].vertices.data()))
			{
				LogError("VertexBuffer create failed!");
				Destroy();
				return false;
			}
			if (!m_subMeshes[i].indexBuffer.Create(RenderResourceUsage::Static, m_subMeshes[i].indices.size(), sizeof(uint32_t), m_subMeshes[i].indices.data()))
			{
				LogError("IndexBuffer create failed!");
				Destroy();
				return false;
			}

			if (!m_subMeshes[i].vao.Create<Vertex_Pos3_TexCoord>(&m_subMeshes[i].vertexBuffer, &m_subMeshes[i].indexBuffer))
			{
				LogError("VAO create failed!");
				Destroy();
				return false;
			}
		}
		return true;
	}

	namespace ModelFileManager
	{
		std::unordered_map<std::string, Model> FileModels;

		void Destroy()
		{
			for (auto it = FileModels.begin(); it != FileModels.end(); ++it)
				it->second.Destroy();
			FileModels.clear();
		}

		Model* LoadModel(const char* name)
		{
			auto it = FileModels.find(name);
			if (it != FileModels.end())
			{
				return &it->second;
			}
			else
			{
				LogPrint("Load model: " + std::string(name));

				Model model;
				if (!model.Create(name) || !model.IsValid())
					return nullptr;

				FileModels[name] = model;
				return &FileModels[name];
			}
		}
	}

	void drawPrimitive::DrawLine(const FreeCamera& camera, const glm::vec3& startPos, const glm::vec3& endPos)
	{
		static bool isCreate = false;
		static VertexArrayBuffer vao;
		static VertexBuffer vertexBuf;
		static ShaderProgram shaderProgram;
		static UniformLocation MatrixID;

		if (!isCreate)
		{
			isCreate = true;

			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};

			vertexBuf.Create(RenderResourceUsage::Dynamic, 2, 3 * sizeof(float), vertexData);

			const std::vector<VertexAttributeRaw> attribs =
			{
				{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, nullptr, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
out vec4 outColor;
void main()
{
outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
		}
		else
		{
			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};
			vertexBuf.Update(0, sizeof(vertexData), vertexData);
		}

		//const glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix()/* * worldMModelMatrixatrix*/;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		glLineWidth(4);
		glDisable(GL_DEPTH_TEST);
		vao.Draw(PrimitiveDraw::Lines);
		glLineWidth(1);
		glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const FreeCamera& camera, const glm::mat4& worldMatrix, const glm::vec4& color, bool disableDepthTest)
	{
		static bool isCreate = false;
		static VertexArrayBuffer vao;
		static VertexBuffer vertexBuf;
		static IndexBuffer indexBuf;
		static ShaderProgram shaderProgram;
		static UniformLocation MatrixID;
		static UniformLocation ColorID;

		if (!isCreate)
		{
			isCreate = true;

			constexpr float vertexData[] =
			{
				-0.5f, -0.5f,  0.5f,// 0
					0.5f, -0.5f,  0.5f,// 1
					0.5f,  0.5f,  0.5f,// 2
				-0.5f,  0.5f,  0.5f,// 3
				-0.5f, -0.5f, -0.5f,// 4
					0.5f, -0.5f, -0.5f,// 5
					0.5f,  0.5f, -0.5f,// 6
				-0.5f,  0.5f, -0.5f,// 7
			};
			constexpr uint16_t indexData[] =
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,
				4, 5,
				5, 6,
				6, 7,
				7, 4,
				3, 7,
				2, 6,
				0, 4,
				1, 5
			};

			vertexBuf.Create(RenderResourceUsage::Static, 8, 3 * sizeof(float), vertexData);
			indexBuf.Create(RenderResourceUsage::Static, 24, sizeof(uint16_t), indexData);

			const std::vector<VertexAttributeRaw> attribs =
			{
				{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, &indexBuf, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
uniform vec4 inColor;
out vec4 outColor;
void main()
{
outColor = inColor;
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
			ColorID = shaderProgram.GetUniformVariable("inColor");
		}

		const glm::mat4 MVP = GetCurrentProjectionMatrix() * camera.GetViewMatrix() * worldMatrix;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		shaderProgram.SetUniform(ColorID, color);
		if (disableDepthTest) glDisable(GL_DEPTH_TEST);
		//glLineWidth(4);
		vao.Draw(PrimitiveDraw::Lines);
		if (disableDepthTest) glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const FreeCamera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotationRadian, const glm::vec4& color, bool disableDepthTest)
	{
		const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), rotationRadian.x, glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), rotationRadian.y, glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), rotationRadian.z, glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 roationMatrix = transformY * transformX * transformZ;

		// translation * rotation * scale (also know as TRS matrix)
		const glm::mat4 ModelMatrix =
			glm::translate(glm::mat4(1.0f), position)
			* roationMatrix
			* glm::scale(glm::mat4(1.0f), size);

		drawPrimitive::DrawCubeWires(camera, ModelMatrix, color, disableDepthTest);
	}

}
#pragma endregion

#pragma region Graphics2D
namespace g2d
{

	class Font
	{
	public:
		uint32_t size = 60;
		std::string fontFileName = "../fonts/OpenSans-Regular.ttf";
		const uint32_t atlasWidth = 1024;
		const uint32_t atlasHeight = 1024;
		//const uint32_t oversampleX = 2;
		//const uint32_t oversampleY =2;
		const uint32_t firstCharENG = ' ';
		const uint32_t charCountENG = '~' - ' ' + 1;
		const uint32_t firstCharRUS = 0x400;
		const uint32_t charCountRUS = 0x452 - 0x400;

		std::unique_ptr<stbtt_packedchar[]> charInfo;
		Texture2D texture;
	};

	static std::vector<Font> m_cacheFont;
	static ShaderProgram cacheShader;
	static UniformLocation m_idAttributeTextColor;
	static UniformLocation m_idAttributeWorldViewProjMatrix;

	constexpr const char* fontVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

uniform mat4 worldViewProjMatrix;

out vec2 uv0;

void main()
{
    gl_Position = worldViewProjMatrix * vec4(vertex.xy, 0.0, 1.0);
    uv0 = vertex.zw;
}
)";
	constexpr const char* fontFragmentShaderSource = R"(
#version 330 core

in vec2 uv0;

uniform sampler2D mainTex;
uniform vec3 textColor;

out vec4 fragColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(mainTex, uv0).r);
    fragColor = vec4(textColor, 1.0) * sampled;
}
)";

	struct GlyphInfo
	{
		glm::vec4 positions[4];
		float offsetX = 0;
		float offsetY = 0;
	};

	inline GlyphInfo makeGlyphInfo(Font* font, uint32_t character, float offsetX, float offsetY)
	{
		stbtt_aligned_quad quad;

		int char_index = 0;
		if (character < font->firstCharENG + font->charCountENG)
			char_index = character - font->firstCharENG;
		else
			char_index = character - font->firstCharRUS + font->charCountENG;

		stbtt_GetPackedQuad(font->charInfo.get(), font->atlasWidth, font->atlasHeight, char_index, &offsetX, &offsetY, &quad, 1);

		const int sizeY = font->size / 2;

		GlyphInfo info{};
		info.offsetX = offsetX;
		info.offsetY = offsetY;
		info.positions[0] = { quad.x0, quad.y0 + sizeY, quad.s0, quad.t0 };
		info.positions[1] = { quad.x0, quad.y1 + sizeY, quad.s0, quad.t1 };
		info.positions[2] = { quad.x1, quad.y1 + sizeY, quad.s1, quad.t1 };
		info.positions[3] = { quad.x1, quad.y0 + sizeY, quad.s1, quad.t0 };

		return info;
	}

	Font* getFont(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = nullptr;
		for (int i = 0; i < m_cacheFont.size(); i++)
		{
			if (m_cacheFont[i].fontFileName == fontFileName && m_cacheFont[i].size == fontSize)
			{
				font = &m_cacheFont[i];
				break;
			}
		}
		if (!font)
		{
			Font font_;
			font_.size = fontSize;
			font_.fontFileName = fontFileName;

			std::ifstream file(fontFileName, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				LogError("Failed to open file " + fontFileName);
				return nullptr;
			}

			const auto size = file.tellg();
			file.seekg(0, std::ios::beg);
			auto bytes = std::vector<uint8_t>(size);
			file.read(reinterpret_cast<char*>(&bytes[0]), size);
			file.close();

			auto atlasData = new uint8_t[font_.atlasWidth * font_.atlasHeight];

			font_.charInfo = std::make_unique<stbtt_packedchar[]>(font_.charCountENG + font_.charCountRUS);

			stbtt_pack_context context;
			//if (!stbtt_PackBegin(&context, atlasData.get(), font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr))
			//	panic("Failed to initialize font");
			stbtt_PackBegin(&context, atlasData, font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr);

			//stbtt_PackSetOversampling(&context, font_.oversampleX, font_.oversampleY);
			//if (!stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get()))
			//    panic("Failed to pack font");

			//stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharENG, font_.charCountENG, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharRUS, font_.charCountRUS, font_.charInfo.get() + font_.charCountENG);

			stbtt_PackEnd(&context);

			Texture2DCreateInfo createInfo;
			createInfo.format = TexelsFormat::R_U8;
			createInfo.width = font_.atlasWidth;
			createInfo.height = font_.atlasHeight;
			createInfo.depth = 1;
			createInfo.pixelData = atlasData;			

			Texture2DInfo textureInfo;
			//textureInfo.minFilter = TextureMinFilter::Linear;
			//textureInfo.magFilter = TextureMagFilter::Linear;
			textureInfo.mipmap = false;

			//font_.texture.Create(GL_RGB, GL_RED, GL_UNSIGNED_BYTE, font_.atlasWidth, font_.atlasHeight, atlasData.get());
			font_.texture.Create(createInfo, textureInfo);

			delete[] atlasData;

			m_cacheFont.push_back(std::move(font_));
			font = &m_cacheFont[m_cacheFont.size() - 1];
		}

		return font;
	}

	bool Text::Create(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = getFont(fontFileName, fontSize);
		if (!font || !create(font))
		{
			LogError("Text not create!");
			return false;
		}

		return true;
	}

	void Text::Destroy()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
	}

	void Text::SetText(const std::wstring& text)
	{
		if (m_font && m_text != text)
		{
			m_text = text;
			std::vector<glm::vec4> vertices;
			std::vector<uint16_t> indexes;

			uint16_t lastIndex = 0;
			float offsetX = 0, offsetY = 0;
			for (auto c : text)
			{
				const auto glyphInfo = makeGlyphInfo(m_font, c, offsetX, offsetY);
				offsetX = glyphInfo.offsetX;
				offsetY = glyphInfo.offsetY;

				vertices.emplace_back(glyphInfo.positions[0]);
				vertices.emplace_back(glyphInfo.positions[1]);
				vertices.emplace_back(glyphInfo.positions[2]);
				vertices.emplace_back(glyphInfo.positions[3]);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 1);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex + 3);

				lastIndex += 4;
			}
			indexElementCount = indexes.size();

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indexElementCount, indexes.data(), GL_STATIC_DRAW);
		}
	}

	void Text::Draw(const glm::vec3& position, const glm::vec3& color, const glm::mat4& orthoMat)
	{
		if (m_text.empty() || !m_font || !m_font->texture.IsValid())
			return;

		const glm::mat4 pm = orthoMat * glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

		cacheShader.Bind();
		cacheShader.SetUniform(m_idAttributeTextColor, { color.x, color.y, color.z });
		cacheShader.SetUniform(m_idAttributeWorldViewProjMatrix, pm);

		m_font->texture.Bind(0);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_TRIANGLES, indexElementCount, GL_UNSIGNED_SHORT, nullptr);
		VertexArrayBuffer::UnBind();
	}

	bool Text::create(Font* font)
	{
		m_font = font;

		if (!cacheShader.IsValid())
		{
			cacheShader.CreateFromMemories(fontVertexShaderSource, fontFragmentShaderSource);
			cacheShader.Bind();
			m_idAttributeTextColor = cacheShader.GetUniformVariable("textColor");
			m_idAttributeWorldViewProjMatrix = cacheShader.GetUniformVariable("worldViewProjMatrix");
			cacheShader.SetUniform("mainTex", 0);
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &indexBuffer);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		return true;
	}

}
#pragma endregion
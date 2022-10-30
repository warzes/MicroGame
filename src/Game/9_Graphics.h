#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"


#pragma region Graphics3D
namespace g3d
{
	class Camera
	{
	public:
		void MoveForward(float deltaTime, float speedMod = 1.0f);
		void MoveBackward(float deltaTime, float speedMod = 1.0f);
		void MoveRight(float deltaTime, float speedMod = 1.0f);
		void MoveLeft(float deltaTime, float speedMod = 1.0f);
		void MoveUp(float deltaTime, float speedMod = 1.0f);
		void MoveDown(float deltaTime, float speedMod = 1.0f);

		void Rotate(float offsetX, float offsetY);

		void SimpleMove(float deltaTime);
		void Update();

		void SetRotate(float yaw, float pitch);
		void SetPosition(const glm::vec3& pos) { m_position = pos; }
		void SetYaw(float val) { m_yaw = val; }
		void SetPitch(float val) { m_pitch = val; }
		void SetSpeed(float val) { m_movementSpeed = val; }
		void SetSensitivity(float val) { m_sensitivity = val; }

		float GetYaw() const { return m_yaw; }
		float GetPitch() const { return m_pitch; }

		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		const glm::vec3& GetPosition() const { return m_position; }
		const glm::vec3& GetDirection() const { return m_front; }
		const glm::vec3& GetRight() const { return m_right; }
		const float GetSpeed() const { return m_movementSpeed; }

		//Frustum ComputeFrustum() const;
		//private:
		void updateVectors();
	private:
		glm::mat4 m_viewMatrix;

		// camera Attributes
		glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_front = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_right;
		glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		// euler Angles
		float m_yaw = -90.0f;
		float m_pitch = 0.0f;

		// camera options
		float m_movementSpeed = 5.0f;
		float m_sensitivity = 0.1f;
	};

	class Material
	{
	public:

		//private:
		Texture2D* diffuseTexture = nullptr;

		glm::vec3 ambientColor = glm::vec3(1.0f);
		glm::vec3 diffuseColor = glm::vec3(1.0f);
		glm::vec3 specularColor = glm::vec3(0.0f);
		float shininess = 1.0f;
	};

	class Mesh
	{
	public:
		std::vector<Vertex_Pos3_TexCoord> vertices;
		std::vector<uint32_t> indices;

		Material material;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		VertexArrayBuffer vao;
	};

	class Model
	{
	public:
		bool Create(const char* fileName, const char* pathMaterialFiles = "./");
		bool Create(std::vector<Mesh>&& m_meshes); // TODO: правильно?
		void Destroy();

		void SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs);

		void Draw(uint32_t instanceCount = 1);
		bool IsValid() const
		{
			if (m_subMeshes.size() > 0)
				return m_subMeshes[0].vertexBuffer.IsValid() && m_subMeshes[0].indexBuffer.IsValid() && m_subMeshes[0].vao.IsValid();
			return false;
		}
		//const std::vector<Vertex_Pos3_TexCoord>& GetVertices() const { return m_vertices; }
		//const std::vector<uint32_t>& GetIndices() const { return m_indices; }

	private:
		bool createBuffer();
		std::vector<Mesh> m_subMeshes;
	};

	namespace ModelFileManager
	{
		void Destroy();
		Model* LoadModel(const char* name);
	}

	namespace drawPrimitive
	{
		void DrawLine(const Camera& camera, const glm::vec3& startPos, const glm::vec3& endPos);

		void DrawCubeWires(const Camera& camera, const glm::mat4& worldMatrix, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec3& size = glm::vec3(1.0f), const glm::vec3& rotationRadian = glm::vec3(0.0f), const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		inline void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec4& color, bool disableDepthTest = false)
		{
			drawPrimitive::DrawCubeWires(camera, position, glm::vec3(1.0f), glm::vec3(0.0f), color, disableDepthTest);
		}
		inline void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, bool disableDepthTest = false)
		{
			drawPrimitive::DrawCubeWires(camera, position, size, glm::vec3(0.0f), color, disableDepthTest);
		}
	}
}
#pragma endregion

#pragma region Graphics2D
namespace g2d
{
	class Font;

	class Text
	{
	public:
		bool Create(const std::string& fontFileName, uint32_t fontSize);
		void Destroy();

		void SetText(const std::wstring& text);
		void Draw(const glm::vec3& position, const glm::vec3& color, const glm::mat4& orthoMat);

	private:
		bool create(Font* font);

		std::wstring m_text;
		Font* m_font = nullptr;
		unsigned vao = 0;
		unsigned vertexBuffer = 0;
		unsigned indexBuffer = 0;
		uint16_t indexElementCount = 0;
		float angle = 0;
	};
}
#pragma endregion
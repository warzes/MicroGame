#pragma once

#include "BaseHeader.h"
#include "oRenderer.h"

// New

struct Camera 
{
	Camera();

	static Camera* GetActive();

	void Teleport(float px, float py, float pz);
	void Move(float incx, float incy, float incz);
	void Fps(float yaw, float pitch);
	void Orbit(float yaw, float pitch, float inc_distance);
	void LookAt(const glm::vec3& target);
	void Enable();


	glm::mat4 m_view = glm::mat4(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_up = glm::vec3(0.0f);
	glm::vec3 m_look = glm::vec3(0.0f);

	float m_yaw = 0.0f, m_pitch = 0.0f, m_speed = 0.0f; // mirror_x, mirror_y;

	// used for friction and smoothing
	glm::vec3 m_last_look = glm::vec3(0.0f);
	glm::vec3 m_last_move = glm::vec3(0.0f);
};

namespace DebugDraw
{
	// -----------------------------------------------------------------------------
	// debugdraw framework
	// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
	// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
	// [x] arrow, point, text, capsule, aabb, plane, flotilla-style locator, boid,
	// [x] line batching
	// [*] line width and stipple
	// [*] (proper) gizmo,
	// [ ] bone (pyramid? two boids?), ring,
	// [ ] camera, light bulb, light probe,

	void DrawPoint(const glm::vec3& from, unsigned rgb);
	void DrawLine(const glm::vec3& from, const glm::vec3& to, unsigned rgb);
	void DrawLineDashed(glm::vec3 from, glm::vec3 to, unsigned rgb);

	void DrawAxis(float units);	
	void DrawGround(float scale);
	void DrawGrid(float scale);

	void DrawTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, unsigned rgb);

	void DrawArrow(const glm::vec3& begin, const glm::vec3& end, unsigned rgb);
	void DrawBounds(const glm::vec3 points[8], unsigned rgb); // TODO: передавать points по ссылке, сейчас оно копируется
	void DrawBox(const glm::vec3& c, const glm::vec3& extents, unsigned rgb);
	void DrawCube(const glm::vec3& center, float radius, unsigned rgb);
	void DrawPlane(const glm::vec3& p, const glm::vec3& n, float scale, unsigned rgb);
	void DrawSphere(const glm::vec3& pos, float radius, unsigned rgb);
	void DrawCapsule(const glm::vec3& from, const glm::vec3& to, float radius, unsigned rgb);

	void DrawDiamond(const glm::vec3& from, const glm::vec3& to, float size, unsigned rgb);
	void DrawPyramid(const glm::vec3& center, float height, int segments, unsigned rgb);
	void DrawPrism(const glm::vec3& center, float radius, float height, const glm::vec3& normal, int segments, unsigned rgb);
	void DrawSquare(const glm::vec3& pos, float radius, unsigned rgb);
	void DrawCylinder(const glm::vec3& center, float height, int segments, unsigned rgb);
	void DrawPentagon(const glm::vec3& pos, float radius, unsigned rgb);
	void DrawHexagon(const glm::vec3& pos, float radius, unsigned rgb);
	void DrawCone(const glm::vec3& center, const glm::vec3& top, float radius, unsigned rgb);
	void DrawCircle(const glm::vec3& pos, const glm::vec3& n, float radius, unsigned rgb);
	void DrawAABB(const glm::vec3& minbb, const glm::vec3& maxbb, unsigned rgb);

	void DrawPosition(const glm::vec3& pos, float radius);
	void DrawPositionDir(const glm::vec3& pos, const glm::vec3& dir, float radius);
	void DrawNormal(const glm::vec3& pos, const glm::vec3& n);
	void DrawBone(const glm::vec3& center, const glm::vec3& end, unsigned rgb);
	void DrawBoid(const glm::vec3& pos, glm::vec3 dir);

	void Flush(const Camera& camera);
}

#pragma region Graphics3D
namespace g3d
{
	class FreeCamera
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
		void SetLook(const glm::vec3& pos) { m_front = pos; }

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
		float m_yaw = 90.0f;
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

	struct MeshCreateInfo
	{
		std::vector<Vertex_Pos3_TexCoord> vertices;
		std::vector<uint32_t> indices;
		Material material;
	};

	class Mesh
	{
	public:
		void Set(MeshCreateInfo&& createInfo)
		{
			vertices = std::move(createInfo.vertices);
			indices = std::move(createInfo.indices);
			material = std::move(createInfo.material);
		}

		Poly GetPoly() const;

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
		bool Create(std::vector<MeshCreateInfo>&& meshes);
		void Destroy();

		void SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs);

		void Draw(uint32_t instanceCount = 1);
		bool IsValid() const
		{
			if (m_subMeshes.size() > 0)
				return m_subMeshes[0].vertexBuffer.IsValid() && m_subMeshes[0].indexBuffer.IsValid() && m_subMeshes[0].vao.IsValid();
			return false;
		}
		//const std::vector<Vertex_Pos3_TexCoord>& GetVertices() const { return m_vertices; }
		//const std::vector<uint32_t>& GetIndices() const { return m_indices; }

		void SetMaterial(const Material& material);

		Poly GetPoly() const;

		std::vector<Mesh>& GetSubMeshes() { return m_subMeshes; }

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
		void DrawLine(const FreeCamera& camera, const glm::vec3& startPos, const glm::vec3& endPos);

		void DrawCubeWires(const FreeCamera& camera, const glm::mat4& worldMatrix, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		void DrawCubeWires(const FreeCamera& camera, const glm::vec3& position, const glm::vec3& size = glm::vec3(1.0f), const glm::vec3& rotationRadian = glm::vec3(0.0f), const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		inline void DrawCubeWires(const FreeCamera& camera, const glm::vec3& position, const glm::vec4& color, bool disableDepthTest = false)
		{
			drawPrimitive::DrawCubeWires(camera, position, glm::vec3(1.0f), glm::vec3(0.0f), color, disableDepthTest);
		}
		inline void DrawCubeWires(const FreeCamera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, bool disableDepthTest = false)
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

		bool IsValid() const { return m_font != nullptr; }

	private:
		bool create(Font* font);

		std::wstring m_text;
		Font* m_font = nullptr;
		uint16_t indexElementCount = 0;
		VertexArrayBuffer m_vao;
		VertexBuffer m_vb;
		IndexBuffer m_ib;
		float angle = 0;
	};
}
#pragma endregion
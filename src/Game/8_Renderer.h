#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

struct RendererCreateInfo
{
	float PerspectiveFOV = 45.0f;
	float PerspectiveNear = 0.01f;
	float PerspectiveFar = 1000.0f;

	glm::vec3 ClearColor = { 0.4f, 0.6f, 1.0f };
};

bool CreateRenderSystem(const RendererCreateInfo& createInfo);
void DestroyRenderSystem();

void BeginRenderFrame();

enum class RenderResourceUsage
{
	Static,
	Dynamic,
	Stream,
};

struct UniformVariable
{
	UniformVariable() = default;
	UniformVariable(int newId) { id = newId; }
	bool IsValid() const { return id > -1; }
	int id = -1;
};

// TODO: юниформы хранящие свой тип данных (и статус изменения)

class ShaderProgram
{
public:
	bool CreateFromMemories(const char* vertexShaderMemory, const char* fragmentShaderMemory);
	void Destroy();

	void Bind();

	static void UnBind();

	UniformVariable GetUniformVariable(const char* name);

	void SetUniform(UniformVariable var, int value);
	void SetUniform(UniformVariable var, float value);
	void SetUniform(UniformVariable var, float x, float y, float z);
	void SetUniform(UniformVariable var, const glm::vec2& v);
	void SetUniform(UniformVariable var, const glm::vec3& v);
	void SetUniform(UniformVariable var, const glm::vec4& v);
	void SetUniform(UniformVariable var, const glm::mat3& mat);
	void SetUniform(UniformVariable var, const glm::mat4& mat);

	void SetUniform(const char* name, int value);
	void SetUniform(const char* name, float value);
	void SetUniform(const char* name, float x, float y, float z);
	void SetUniform(const char* name, const glm::vec2& v);
	void SetUniform(const char* name, const glm::vec3& v);
	void SetUniform(const char* name, const glm::vec4& v);
	void SetUniform(const char* name, const glm::mat3& mat);
	void SetUniform(const char* name, const glm::mat4& mat);

	bool IsValid() const { return m_id > 0; }

private:
	unsigned createShader(unsigned type, const char* source) const;

	unsigned m_id = 0;
};

enum class TextureMinFilter
{
	Nearest,
	Linear,
	NearestMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapNearest,
	LinearMipmapLinear,
};

enum class TextureMagFilter
{
	Nearest,
	Linear,
};

enum class TextureWrapping
{
	Repeat,
	MirroredRepeat,
	Clamp,
};

enum class TexelsFormat
{
	None = 0,
	R_U8,
	RG_U8,
	RGB_U8,
	RGBA_U8,
	Depth_U16,
	DepthStencil_U16,
	Depth_U24,
	DepthStencil_U24,
};

struct Texture2DLoaderInfo
{
	RenderResourceUsage usage = RenderResourceUsage::Static;

	TextureMinFilter minFilter = TextureMinFilter::NearestMipmapNearest;
	TextureMagFilter magFilter = TextureMagFilter::Nearest;
	TextureWrapping wrapS = TextureWrapping::Repeat;
	TextureWrapping wrapT = TextureWrapping::Repeat;
	TextureWrapping wrapR = TextureWrapping::Repeat;

	const char* fileName = nullptr;
	bool verticallyFlip = true;
	bool mipmap = true;
};

struct Texture2DCreateInfo
{
	Texture2DCreateInfo() = default;
	Texture2DCreateInfo(const Texture2DLoaderInfo& loaderInfo)
	{
		usage = loaderInfo.usage;
		minFilter = loaderInfo.minFilter;
		magFilter = loaderInfo.magFilter;
		wrapS = loaderInfo.wrapS;
		wrapT = loaderInfo.wrapT;
		wrapR = loaderInfo.wrapR;
		mipmap = loaderInfo.mipmap;
	}

	RenderResourceUsage usage = RenderResourceUsage::Static;

	TextureMinFilter minFilter = TextureMinFilter::NearestMipmapNearest;
	TextureMagFilter magFilter = TextureMagFilter::Nearest;
	TextureWrapping wrapS = TextureWrapping::Repeat;
	TextureWrapping wrapT = TextureWrapping::Repeat;
	TextureWrapping wrapR = TextureWrapping::Repeat;

	TexelsFormat format = TexelsFormat::RGBA_U8;
	uint16_t width = 1;
	uint16_t height = 1;
	uint16_t depth = 1;
	uint8_t* data = nullptr;
	unsigned mipMapCount = 1; // TODO: only compressed
	bool mipmap = true;
	bool isTransparent = false;
};

class Texture2D
{
public:
	bool CreateFromMemories(const Texture2DCreateInfo& createInfo);
	bool CreateFromFiles(const Texture2DLoaderInfo& loaderInfo);

	void Destroy();

	void Bind(unsigned slot = 0) const;

	static void UnBind(unsigned slot = 0);

	bool IsValid() const { return id > 0; }

	unsigned id = 0;
	bool isTransparent = false;
};

class VertexBuffer
{
public:
	bool Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data);
	void Destroy();

	void Update(unsigned offset, unsigned size, const void* data);

	void Bind() const;

	unsigned GetVertexCount() const { return m_vertexCount; }

	bool IsValid() const { return m_id > 0; }

private:
	RenderResourceUsage m_usage = RenderResourceUsage::Static;
	unsigned m_id = 0;
	unsigned m_vertexCount = 0;
	unsigned m_vertexSize = 0;
};

class IndexBuffer
{
public:
	bool Create(RenderResourceUsage usage, unsigned indexCount, unsigned indexSize, const void* data);
	void Destroy();

	void Bind() const;

	unsigned GetIndexCount() const { return m_indexCount; }
	unsigned GetIndexSize() const { return m_indexSize; }

	bool IsValid() const { return m_id > 0; }

private:
	RenderResourceUsage m_usage = RenderResourceUsage::Static;
	unsigned m_id = 0;
	unsigned m_indexCount = 0;
	unsigned m_indexSize = 0;
};

enum class VertexAttributeType
{
	Float,
	Matrix4
};

struct VertexAttribute
{
	unsigned size;       // ignore in instanceAttr
	VertexAttributeType type;
	bool normalized;
	unsigned stride;     // = sizeof Vertex, ignore in instanceAttr
	const void* pointer; // (void*)offsetof(Vertex, tex_coord)}, ignore in instanceAttr
};

enum class PrimitiveDraw
{
	Lines,
	Triangles,
	Points,
};

class VertexArrayBuffer
{
public:
	bool Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs);
	bool Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs, const std::vector<VertexAttribute>& instanceAttribs);
	bool Create(const std::vector<VertexBuffer*> vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs);

	void Destroy();

	// есть два варианта инстансинга
	//	1) массив юниформ. Недостаток: кол-во юниформ ограничено и зависит от железа
	//		в этом случае в шейдере нужно создать массив юниформ, например uniform mat4 MVPs[200];
	//		обращение в коде шейдера: gl_Position = MVPs[gl_InstanceID] * vec4(aPos, 1.0);
	//		записать значения shader3.SetUniform(("MVPs[" + std::to_string(i) + "]").c_str(), MVP);
	//		в VAO в Draw() отрисовать нужное число model.Draw(200); 
	//	
	//	2) инстансбуффер. Позволит больше отрисовать.
	//		в шейдере прописываются атрибуты layout(location = 2) in mat4 instanceMatrix;
	//		работа: gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
	//		создается вершинный буфер с кол-вом инстансом - instancevb.Create(Static, amount, sizeof(glm::mat4), &modelMatrices[0]);
	//		в вао вызывается SetInstancedBuffer().
	//		в Draw можно указать кол-во инстансов (но это не обязательно)
	void SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs);


	void Draw(PrimitiveDraw primitive = PrimitiveDraw::Triangles, uint32_t instanceCount = 1);
	void DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex);

	bool IsValid() const { return m_id > 0; }

	static void UnBind();

private:
	unsigned m_id = 0;
	VertexBuffer* m_vbo = nullptr;
	VertexBuffer* m_instanceBuffer = nullptr;
	IndexBuffer* m_ibo = nullptr;
	unsigned m_attribsCount = 0;
	unsigned m_instancedAttribsCount = 0;

};

class FrameBuffer
{
public:
	bool Create(int width, int height);
	void Destroy();

	void Bind(const glm::vec3& color);

	void BindTextureBuffer();

	static void MainFrameBufferBind();

	bool IsValid() const { return m_id > 0 && m_texColorBuffer > 0 && m_rbo > 0; }
	const glm::mat4& GetProjectionMatrix() { return m_projectionMatrix; }

private:
	unsigned m_id = 0;
	unsigned m_texColorBuffer = 0;
	unsigned m_rbo = 0;
	int m_width = 0;
	int m_height = 0;
	glm::mat4 m_projectionMatrix;
};

const glm::mat4& GetCurrentProjectionMatrix();

struct Vertex_Pos2
{
	glm::vec2 position;
};

struct Vertex_Pos2_TexCoord
{
	glm::vec2 position;
	glm::vec2 texCoord;
};

struct Vertex_Pos2_Color
{
	glm::vec2 position;
	glm::vec3 color;
};

struct Vertex_Pos3
{
	glm::vec3 position;
};

struct Vertex_Pos3_TexCoord
{
	bool operator==(const Vertex_Pos3_TexCoord& other) const { return position == other.position && texCoord == other.texCoord; }

	glm::vec3 position;
	glm::vec2 texCoord;
};

struct Vertex_Pos3_Normal_TexCoord
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

template<typename T> std::vector<VertexAttribute> GetVertexAttributes();

template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2>()
{
	using T = Vertex_Pos2;
	return
	{
		{.size = 2, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)}
	};
}

template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2_TexCoord>()
{
	using T = Vertex_Pos2_TexCoord;
	return
	{
		{.size = 2, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)}
	};
}

template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2_Color>()
{
	using T = Vertex_Pos2_Color;
	return
	{
		{.size = 2, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 3, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, color)}
	};
}

template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos3>()
{
	using T = Vertex_Pos3;
	return
	{
		{.size = 3, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)}
	};
}
template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos3_TexCoord>()
{
	using T = Vertex_Pos3_TexCoord;
	return
	{
		{.size = 3, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeType::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)}
	};
}


namespace TextureFileManager
{
	void Destroy();
	Texture2D* LoadTexture2D(const char* name);
	Texture2D* LoadTexture2D(const Texture2DLoaderInfo& loaderInfo);
}
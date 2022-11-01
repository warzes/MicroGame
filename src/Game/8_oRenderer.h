#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"
#include "8_Renderer.h"

//=============================================================================
// TODO:
// - иды в ресурсах - возможно передать так uint16_t idx = kInvalidHandle; uint16_t kInvalidHandle = UINT16_MAX;
//=============================================================================


//=============================================================================
// Vertex Attributes
//=============================================================================

enum class VertexAttribute
{
	// Corresponds to vertex shader attribute.
	Position,  // a_position
	Normal,    // a_normal
	Tangent,   // a_tangent
	Bitangent, // a_bitangent
	Color0,    // a_color0
	Color1,    // a_color1
	Color2,    // a_color2
	Color3,    // a_color3
	Indices,   // a_indices
	Weight,    // a_weight
	TexCoord0, // a_texcoord0
	TexCoord1, // a_texcoord1
	TexCoord2, // a_texcoord2
	TexCoord3, // a_texcoord3
	TexCoord4, // a_texcoord4
	TexCoord5, // a_texcoord5
	TexCoord6, // a_texcoord6
	TexCoord7, // a_texcoord7
	TexCoord8, // a_texcoord8
	TexCoord9, // a_texcoord9
	TexCoord10, // a_texcoord10
	TexCoord11, // a_texcoord11
	TexCoord12, // a_texcoord12
	TexCoord13, // a_texcoord13
	TexCoord14, // a_texcoord14
	TexCoord15, // a_texcoord15

	Count
};

// Vertex attribute type enum.
enum class VertexAttributeType
{
	Uint8,
	Uint10,
	Int16,
	Half,
	Float,

	Count
};

enum class VertexAttributeTypeRaw
{
	Float,
	Matrix4
};

struct VertexAttributeRaw
{
	unsigned size;       // ignore in instanceAttr
	VertexAttributeTypeRaw type;
	bool normalized;
	unsigned stride;     // = sizeof Vertex, ignore in instanceAttr
	const void* pointer; // (void*)offsetof(Vertex, tex_coord)}, ignore in instanceAttr
};

class VertexAttributesRaw
{
public:
	// TODO: std::vector<VertexAttributeRaw> arrays
	// также сделать возможность получения идов с шейдера (при добавлении атрибута можно указать шейдер и имя)
};



/*
EXAMPLE:

VertexLayout layout;
layout
	.Begin()
	.Add(VertexAttribute::Position, 3, VertexAttributeType::Float)
	.Add(VertexAttribute::Color0,   4, VertexAttributeType::Uint8, true)
	.End();
*/
class VertexLayout
{
public:
	VertexLayout& Begin();
	void End();

	// Add attribute to VertexLayout.
	/// @param[in] _attrib Attribute semantics.
	/// @param[in] _num Number of elements 1, 2, 3 or 4.
	/// @param[in] _type Element type.
	/// @param[in] _normalized When using fixed point AttribType (f.e. Uint8)
	///   value will be normalized for vertex shader usage. When normalized
	///   is set to true, AttribType::Uint8 value in range 0-255 will be
	///   in range 0.0-1.0 in vertex shader.
	/// @param[in] _asInt Packaging rule for vertexPack, vertexUnpack, and
	///   vertexConvert for AttribType::Uint8 and AttribType::Int16.
	///   Unpacking code must be implemented inside vertex shader.
	/// @returns Returns itself.
	///
	/// @remarks
	///   Must be called between begin/end.
	VertexLayout& Add(VertexAttribute attrib, uint8_t num, VertexAttributeType type, bool normalized = false, bool asInt = false);

	// Skip _num bytes in vertex stream.
	VertexLayout& Skip(uint8_t _num);

	// Decode attribute.
	void Decode(VertexAttribute attrib, uint8_t& num, VertexAttributeType& type, bool& normalized, bool& asInt) const;

	// Returns `true` if VertexLayout contains attribute.
	bool Has(VertexAttribute attrib) const { return UINT16_MAX != m_attributes[static_cast<size_t>(attrib)]; }

	// Returns relative attribute offset from the vertex.
	uint16_t GetOffset(VertexAttribute attrib) const { return m_offset[static_cast<size_t>(attrib)]; }

	// Returns vertex stride.
	uint16_t getStride() const { return m_stride; }

	/// Returns size of vertex buffer for number of vertices.
	uint32_t GetSize(uint32_t num) const { return num * m_stride; }
private:
	uint32_t m_hash = 0;
	uint16_t m_stride = 0;
	uint16_t m_offset[static_cast<size_t>(VertexAttribute::Count)] = { 0 };
	uint16_t m_attributes[static_cast<size_t>(VertexAttribute::Count)] = { 0 };
};

//=============================================================================
// Render System
//=============================================================================

namespace RenderSystem
{
	struct CreateInfo
	{
		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 1000.0f;

		glm::vec3 ClearColor = { 0.4f, 0.6f, 1.0f };
	};

	bool Create(const CreateInfo& createInfo);
	void Destroy();

	void BeginFrame();
}





enum class RenderResourceUsage
{
	Static,
	Dynamic,
	Stream,
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



enum class PrimitiveDraw
{
	Lines,
	Triangles,
	Points,
};

class VertexArrayBuffer
{
public:
	bool Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs);
	bool Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs, const std::vector<VertexAttributeRaw>& instanceAttribs);
	bool Create(const std::vector<VertexBuffer*>& vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs);

	template<typename T>
	bool Create(VertexBuffer* vbo, IndexBuffer* ibo)
	{
		return Create(vbo, ibo, GetVertexAttributes<T>());
	}

	bool Create(VertexBuffer* vbo, IndexBuffer* ibo, ShaderProgram* shaders);

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
	void SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs);


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

template<typename T> std::vector<VertexAttributeRaw> GetVertexAttributes();

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos2>()
{
	using T = Vertex_Pos2;
	return
	{
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)}
	};
}

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos2_TexCoord>()
{
	using T = Vertex_Pos2_TexCoord;
	return
	{
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)}
	};
}

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos2_Color>()
{
	using T = Vertex_Pos2_Color;
	return
	{
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, color)}
	};
}

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos3>()
{
	using T = Vertex_Pos3;
	return
	{
		{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)}
	};
}
template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos3_TexCoord>()
{
	using T = Vertex_Pos3_TexCoord;
	return
	{
		{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)}
	};
}


namespace TextureFileManager
{
	void Destroy();
	Texture2D* LoadTexture2D(const char* name);
	Texture2D* LoadTexture2D(const Texture2DLoaderInfo& loaderInfo);
}
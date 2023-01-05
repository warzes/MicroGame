#pragma once

#include "BaseHeader.h"

//=============================================================================
// TODO:
// - иды в ресурсах - возможно передать так uint16_t idx = kInvalidHandle; uint16_t kInvalidHandle = UINT16_MAX;
//=============================================================================

//=============================================================================
// RenderCore
//=============================================================================

enum class RenderResourceUsage
{
	Static,
	Dynamic,
	Stream,
};

//=============================================================================
// RenderState
//=============================================================================
struct RenderState
{

};

class BlendState
{
public:

};

//=============================================================================
// ShaderProgram
//=============================================================================

struct UniformLocation
{
	UniformLocation() = default;
	UniformLocation(int newId) { id = newId; }

	bool operator==(const UniformLocation& var) { return id == var.id; }

	bool IsValid() const { return id > -1; }
	int id = -1;
};

struct ShaderAttribInfo
{
	std::string GetText() const { return typeName + " " + name + " is at location " + std::to_string(location); }
	// TODO: сохранять тип атрибута в виде перечисления (чтобы потом можно было создать вертекатрибут)
	std::string typeName;
	unsigned typeId;
	std::string name;
	int location;
};

struct ShaderUniformInfo
{
	std::string GetText() const { return ""; }
};

enum class ShaderType
{
	Vertex,
	Geometry,
	Fragment
};

// TODO: юниформы хранящие свой тип данных (и статус изменения)
class ShaderProgram
{
public:
	bool CreateFromMemories(const std::string& vertexShaderMemory, const std::string& fragmentShaderMemory);
	bool CreateFromMemories(const std::string& vertexShaderMemory, const std::string& geometryShaderMemory, const std::string& fragmentShaderMemory);
	void Destroy();

	void Bind();
	static void UnBind();

	// Shader Program Info
	[[nodiscard]] std::vector<ShaderAttribInfo> GetAttribInfo() const;
	[[nodiscard]] std::vector<ShaderUniformInfo> GetUniformInfo() const;

	// Program interfaces.
#if OPENGL_VERSION >= 43
	[[nodiscard]] int GetInterfaceActiveResources(const unsigned interface) const;
	[[nodiscard]] int GetInterfaceMaxNameLength(const unsigned interface) const;
	[[nodiscard]] int GetInterfaceMaxActiveVariableCount(const unsigned interface) const;
	[[nodiscard]] int GetInterfaceMaxCompatibleSubroutineCount(const unsigned interface) const;
	[[nodiscard]] unsigned GetResourceIndex(const unsigned interface, const std::string& name) const;
	[[nodiscard]] std::string GetResourceName(const unsigned interface, const unsigned index) const;
	[[nodiscard]] int GetResourceLocation(const unsigned interface, const std::string& name) const;
	[[nodiscard]] int GetResourceLocationIndex(const unsigned interface, const std::string& name) const;
	[[nodiscard]] int GetResourceParameter(const unsigned interface, const unsigned index, unsigned parameter) const;
	[[nodiscard]] std::vector<int> GetResourceParameters(const unsigned interface, const unsigned index, const std::vector<unsigned>& parameters) const;
#endif

	// Program binaries.
#if OPENGL_VERSION >= 41
	std::vector<GLbyte> GetProgramBinary(unsigned format) const;
	void SetProgramBinary(const unsigned format, const std::vector<GLbyte>& binary);
#endif

	// Uniform variables
	[[nodiscard]] int GetUniformLocation(const char* name) const;
	[[nodiscard]] UniformLocation GetUniformVariable(const char* name) const;
	[[nodiscard]] std::string GetActiveUniformName(const unsigned index) const;
	[[nodiscard]] unsigned GetUniformIndex(const char* name) const;
	[[nodiscard]] std::vector<unsigned> GetUniformIndices(const int count, const std::vector<std::string>& names) const;
	[[nodiscard]] std::tuple<std::string, unsigned, int> GetActiveUniform(const unsigned index) const;
	[[nodiscard]] int GetActiveUniformNameLength(const unsigned index) const;
	[[nodiscard]] unsigned GetActiveUniformGLType(const unsigned index) const;
	[[nodiscard]] std::vector<unsigned> GetActiveUniformsGLTypes(const std::vector<unsigned>& indices) const;
	[[nodiscard]] unsigned GetActiveUniformOffset(const unsigned index) const;
	[[nodiscard]] std::vector<unsigned> GetActiveUniformsOffsets(const std::vector<unsigned>& indices) const;
	[[nodiscard]] int GetActiveUniformSize(const unsigned index) const;
	[[nodiscard]] std::vector<int> GetActiveUniformsSizes(const std::vector<unsigned>& indices) const;
	[[nodiscard]] unsigned GetActiveUniformBlockIndex(const unsigned index) const;
	[[nodiscard]] std::vector<unsigned> GetActiveUniformsBlockIndices(const std::vector<unsigned>& indices) const;
	[[nodiscard]] int GetActiveUniformArrayStride(const unsigned index) const;
	[[nodiscard]] std::vector<int> GetActiveUniformsArrayStrides(const std::vector<unsigned>& indices) const;
	[[nodiscard]] int GetActiveUniformMatrixStride(const unsigned index) const;
	[[nodiscard]] std::vector<int> GetActiveUniformsMatrixStrides(const std::vector<unsigned>& indices) const;
	[[nodiscard]] bool GetActiveUniformIsRowMajor(const unsigned index) const;
	[[nodiscard]] unsigned GetActiveUniformAtomicCounterBufferIndex(const unsigned index) const;
	[[nodiscard]] std::vector<unsigned> GetActiveUniformsAtomicCounterBufferIndices(const std::vector<unsigned>& indices) const;
	[[nodiscard]] std::vector<int> GetActiveUniformsNameLengths(const std::vector<unsigned>& indices) const;

	// Uniform block info
	[[nodiscard]] unsigned GetUniformBlockIndex(const char* name) const;
	[[nodiscard]] std::string GetActiveUniformBlockName(const unsigned index) const;
	[[nodiscard]] unsigned GetActiveUniformBlockBinding(const unsigned index) const;
	[[nodiscard]] unsigned GetActiveUniformBlockDataSize(const unsigned index) const;
	[[nodiscard]] unsigned GetActiveUniformBlockNameLength(const unsigned index) const;
	[[nodiscard]] int GetActiveUniformBlockUniformCount(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByVertexShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByFragmentShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByComputeShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByGeometryShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByTessellationControlShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveUniformBlockUniformIsReferencedByTessellationEvaluationShader(const unsigned index) const;
	[[nodiscard]] std::vector<unsigned> GetActiveUniformBlockUniformIndices(const unsigned index) const;
#if OPENGL_VERSION >= 42
	[[nodiscard]] unsigned GetActiveAtomicCounterBufferDataSize(const unsigned index) const;
	[[nodiscard]] int GetActiveAtomicCounterBufferCounters(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByVertexShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByFragmentShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByComputeShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByGeometryShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByTessellationControlShader(const unsigned index) const;
	[[nodiscard]] bool GetActiveAtomicCounterBufferIsReferencedByTessellationEvaluationShader(const unsigned index) const;
#endif
	[[nodiscard]] std::vector<unsigned> GetActiveAtomicCounterBufferCounterIndices(const unsigned index) const;

	// Set Uniform
	void SetUniform(UniformLocation var, int value);
	void SetUniform(UniformLocation var, float value);
	void SetUniform(UniformLocation var, float x, float y, float z);
	void SetUniform(UniformLocation var, const glm::vec2& v);
	void SetUniform(UniformLocation var, const glm::vec3& v);
	void SetUniform(UniformLocation var, const glm::vec4& v);
	void SetUniform(UniformLocation var, const glm::mat3& mat);
	void SetUniform(UniformLocation var, const glm::mat4& mat);
	void SetUniform(const char* name, int value) { SetUniform(GetUniformVariable(name), value);	}
	void SetUniform(const char* name, float value) { SetUniform(GetUniformVariable(name), value); }
	void SetUniform(const char* name, float x, float y, float z) { SetUniform(GetUniformVariable(name), x, y, z); }
	void SetUniform(const char* name, const glm::vec2& v) { SetUniform(GetUniformVariable(name), v); }
	void SetUniform(const char* name, const glm::vec3& v) { SetUniform(GetUniformVariable(name), v); }
	void SetUniform(const char* name, const glm::vec4& v) { SetUniform(GetUniformVariable(name), v); }
	void SetUniform(const char* name, const glm::mat3& mat) { SetUniform(GetUniformVariable(name), mat); }
	void SetUniform(const char* name, const glm::mat4& mat) { SetUniform(GetUniformVariable(name), mat); }

	void SetUniformBlockBinding(const unsigned index, const unsigned binding) const;
#if OPENGL_VERSION >= 43
	void SetShaderStorageBlockBinding(const unsigned index, const unsigned binding) const;
#endif

	// Info
	[[nodiscard]] int GetActiveAttributeCount() const;
	[[nodiscard]] int GetActiveAttributeMaxLength() const;
	[[nodiscard]] int GetActiveUniformCount() const;
	[[nodiscard]] int GetActiveUniformMaxLength() const;
	[[nodiscard]] int GetActiveUniformBlockCount() const;
	[[nodiscard]] int GetActiveUniformBlockMaxNameLength() const;
	[[nodiscard]] unsigned GetTransformFeedbackBufferMode() const;
	[[nodiscard]] int GetTransformFeedbackVaryingCount() const;
	[[nodiscard]] int GetTransformFeedbackVaryingMaxLength() const;
	[[nodiscard]] int GetGeometryVerticesOut() const;
	[[nodiscard]] unsigned GetGeometryInputType() const;
	[[nodiscard]] unsigned GetGeometryOutputType() const;
	[[nodiscard]] int GetGeometryShaderInvocations() const;
	[[nodiscard]] int GetTessellationControlOutputVertexCount() const;
	[[nodiscard]] unsigned GetTessellationGenerationMode() const;
	[[nodiscard]] unsigned GetTessellationGenerationSpacing() const;
	[[nodiscard]] unsigned GetTessellationGenerationVertexOrder() const;
	[[nodiscard]] bool GetTessellationGenerationPointMode() const;
	[[nodiscard]] bool IsSeparable() const;
	[[nodiscard]] bool IsBinaryRetrievable() const;
	[[nodiscard]] int GetActiveAtomicCounterBufferCount() const;
	[[nodiscard]] int GetBinaryLength() const;

	// Vertex attributes.
	void SetAttributeLocation(const std::string& attributeName, const unsigned location) const;
	[[nodiscard]] unsigned GetAttributeLocation(const std::string& attributeName) const;
	[[nodiscard]] std::tuple<std::string, unsigned, int> GetActiveAttribute(const unsigned location) const;

	// Transform feedback variables.
	void SetTransformFeedbackVaryings(const std::vector<std::string>& varyings, const unsigned bufferMode);
	[[nodiscard]] std::tuple<std::string, unsigned, int> GetTransformFeedbackVarying(const unsigned index) const;

	// Fragment shaders.
	void SetFragDataLocation(const unsigned colorNumber, const std::string& name) const;
	void SetFragDatLocationIndexed(const unsigned colorNumber, const unsigned index, const std::string& name) const;
	[[nodiscard]] GLuint GetFragDataLocation(const std::string& name) const;
	[[nodiscard]] GLuint GetFragDataIndex(const std::string& name) const;


	[[nodiscard]] bool IsValid() const { return m_id > 0; }	
	[[nodiscard]] bool IsSlowValid() const;

	bool operator==(const ShaderProgram&) const = default;

private:
	[[nodiscard]] unsigned createShader(ShaderType type, const std::string& source) const;

	[[nodiscard]] int getParameter(const unsigned parameter) const;
#if OPENGL_VERSION >= 43
	[[nodiscard]] int getInterfaceParameter(const unsigned interface, const unsigned parameter) const; // ONLY OpenGL4.3/OpenGLES3
#endif
	[[nodiscard]] int getActiveUniformParameter(const unsigned index, const unsigned parameter) const;
	[[nodiscard]] std::vector<int> getActiveUniformParameters(const std::vector<unsigned>& indices, const unsigned parameter) const;
	[[nodiscard]] int getActiveUniformBlockParameter(const unsigned index, const unsigned parameter) const;
#if OPENGL_VERSION >= 42
	[[nodiscard]] int getActiveAtomicCounterBufferParameter(const unsigned index, const unsigned parameter) const; // ONLY OpenGL4.2
#endif
#if OPENGL_VERSION >= 40
	[[nodiscard]] int getProgramStageParameter(const unsigned shaderPype, const unsigned parameter) const; // ONLY OpenGL4.0
#endif

	unsigned m_id = 0;
};

namespace ShaderLoader
{
	void Destroy();
	ShaderProgram* Load(const char* fileName);

	bool IsLoad(const ShaderProgram& shaderProgram);
}

//=============================================================================
// Image
//=============================================================================

enum class ImagePixelFormat
{
	FromSource,
	R_U8,
	RG_U8,
	RGB_U8,
	RGBA_U8
};
class Image
{
public:
	Image() = default;
	Image(Image&&) noexcept;
	~Image();
	Image& operator=(Image&&) noexcept;

	// если pixelData = null то создает белый Image
	bool Create(uint16_t width, uint16_t height, uint8_t channels, const std::vector<uint8_t>& pixelData);
	bool Load(const char* fileName, ImagePixelFormat desiredFormat = ImagePixelFormat::FromSource, bool verticallyFlip = false);
	void Destroy();

	bool IsValid() const { return !m_pixels.empty() && m_width > 0 && m_height > 0 && m_comps > 0 && m_comps <= 4; }

	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	uint16_t GetChannels() const { return m_comps; }
	uint8_t* GetData() { return m_pixels.data(); }
	const uint8_t* GetData() const { return m_pixels.data(); }

	size_t GetSizeData() const { return m_pixels.size(); }

	uint8_t& operator[](size_t idx) { return m_pixels[idx]; }
	const uint8_t& operator[](size_t idx) const { return m_pixels[idx]; }

	bool IsTransparent() const;

	// bilinear interpolation (uv must be in image coords, range [0..w-1,0..h-1])
	glm::vec3 Bilinear(const glm::vec2& uv);

private:
	// TODO: сделать операторы копирования
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	void moveData(Image&& imageRef);

	uint16_t m_width = 0;
	uint16_t m_height = 0;
	uint8_t m_comps = 0;
	std::vector<uint8_t> m_pixels;
};

namespace ImageLoader
{
	void Destroy();
	Image* Load(const char* fileName);

	bool IsLoad(const Image& image);
}

//=============================================================================
// Texture
//=============================================================================

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

struct Texture2DInfo
{
	RenderResourceUsage usage = RenderResourceUsage::Static;

	TextureMinFilter minFilter = TextureMinFilter::NearestMipmapNearest;
	TextureMagFilter magFilter = TextureMagFilter::Nearest;
	TextureWrapping wrapS = TextureWrapping::Repeat;
	TextureWrapping wrapT = TextureWrapping::Repeat;
	TextureWrapping wrapR = TextureWrapping::Repeat;

	bool mipmap = true;
};

struct Texture2DCreateInfo
{
	TexelsFormat format = TexelsFormat::RGBA_U8;
	uint16_t width = 1;
	uint16_t height = 1;
	uint16_t depth = 1;
	uint8_t* pixelData = nullptr;
	unsigned mipMapCount = 1; // TODO: only compressed

	bool isTransparent = false;
};

class Texture2D
{
public:
	bool Create(const char* fileName, bool verticallyFlip = true, const Texture2DInfo& textureInfo = {});
	bool Create(Image* image, const Texture2DInfo& textureInfo = {});
	bool Create(const Texture2DCreateInfo& createInfo, const Texture2DInfo& textureInfo = {});

	void Destroy();

	void Bind(unsigned slot = 0) const;

	static void UnBind(unsigned slot = 0);

	unsigned GetWidth() const { return m_width; }
	unsigned GetHeight() const { return m_height; }

	bool IsValid() const { return m_id > 0; }

	bool operator==(const Texture2D&) const = default;

	bool isTransparent = false;

private:
	unsigned m_id = 0;
	unsigned m_width = 0;
	unsigned m_height = 0;
};

namespace TextureLoader
{
	void Destroy();
	Texture2D* LoadTexture2D(const char* name, bool verticallyFlip = true, const Texture2DInfo& textureInfo = {});

	bool IsLoad(const Texture2D& texture);
}

//=============================================================================
// VertexBuffer
//=============================================================================

class VertexBuffer
{
public:
	// TODO: возможность создания буфера без данных
	bool Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data);
	void Destroy();

	void Update(unsigned offset, unsigned vertexCount, unsigned vertexSize, const void* data);

	void Bind() const;

	unsigned GetVertexCount() const { return m_vertexCount; }

	bool IsValid() const { return m_id > 0; }

private:
	RenderResourceUsage m_usage = RenderResourceUsage::Static;
	unsigned m_id = 0;
	unsigned m_vertexCount = 0;
	unsigned m_vertexSize = 0;
};

//=============================================================================
// IndexBuffer
//=============================================================================

class IndexBuffer
{
public:
	// TODO: возможность создания буфера без данных
	bool Create(RenderResourceUsage usage, unsigned indexCount, unsigned indexSize, const void* data);
	void Destroy();

	void Bind() const;

	void Update(unsigned offset, unsigned indexCount, unsigned indexSize, const void* data);

	unsigned GetIndexCount() const { return m_indexCount; }
	unsigned GetIndexSize() const { return m_indexSize; }

	bool IsValid() const { return m_id > 0; }

private:
	RenderResourceUsage m_usage = RenderResourceUsage::Static;
	unsigned m_id = 0;
	unsigned m_indexCount = 0;
	unsigned m_indexSize = 0;
};

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

//=============================================================================
// VertexArrayBuffer
//=============================================================================

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

	VertexBuffer* GetVertexBuffer() { return m_vbo; }
	IndexBuffer* GetIndexBuffer() { return m_ibo; }

private:
	unsigned m_id = 0;
	VertexBuffer* m_vbo = nullptr;
	VertexBuffer* m_instanceBuffer = nullptr;
	IndexBuffer* m_ibo = nullptr;
	unsigned m_attribsCount = 0;
	unsigned m_instancedAttribsCount = 0;
};

//=============================================================================
// FrameBuffer
//=============================================================================

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
	bool checkFramebuffer();

	unsigned m_id = 0;
	unsigned m_texColorBuffer = 0;
	unsigned m_rbo = 0;
	int m_width = 0;
	int m_height = 0;
	glm::mat4 m_projectionMatrix;
};

const glm::mat4& GetCurrentProjectionMatrix();

//=============================================================================
// Vertex Attributes
//=============================================================================

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

struct Vertex_Pos2_TexCoord_Color
{
	glm::vec2 position;
	glm::vec2 texCoord;
	glm::vec3 color;
};

struct Vertex_Pos2_TexCoord_Color4
{
	glm::vec2 position;
	glm::vec2 texCoord;
	glm::vec4 color;
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

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos2_TexCoord_Color>()
{
	using T = Vertex_Pos2_TexCoord_Color;
	return
	{
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)},
		{.size = 3, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, color)}
	};
}

template<> inline std::vector<VertexAttributeRaw> GetVertexAttributes<Vertex_Pos2_TexCoord_Color4>()
{
	using T = Vertex_Pos2_TexCoord_Color4;
	return
	{
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, position)},
		{.size = 2, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, texCoord)},
		{.size = 4, .type = VertexAttributeTypeRaw::Float, .normalized = false, .stride = sizeof(T), .pointer = (void*)offsetof(T, color)}
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

	void SetFrameColor(const glm::vec3 clearColor);

	void BeginFrame();
}
#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

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

	[[nodiscard]] std::vector<ShaderAttribInfo> GetAttribInfo() const;
	[[nodiscard]] std::vector<ShaderUniformInfo> GetUniformInfo() const;

	[[nodiscard]] UniformLocation GetUniformVariable(const char* name);

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

	[[nodiscard]] bool IsValid() const { return m_id > 0; }

	bool operator==(const ShaderProgram&) const = default;

private:
	unsigned createShader(ShaderType type, const std::string& source) const;

	unsigned m_id = 0;
};

namespace ShaderLoader
{
	void Destroy();
	ShaderProgram* Load(const char* name);

	bool IsLoad(const ShaderProgram& shaderProgram);
}

//=============================================================================
// Image
//=============================================================================
// TODO: может куда-то еще? например в Platform
class Image
{
public:


	unsigned w;
	unsigned h;
	unsigned comps;
	uint8_t* pixels;
};

посмотреть
https ://www.youtube.com/watch?v=oBTJdZo1a2Y&t=145s

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
	uint8_t* pixelData = nullptr;
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
	Texture2D* LoadTexture2D(const char* name);
	Texture2D* LoadTexture2D(const Texture2DLoaderInfo& loaderInfo);

	bool IsLoad(const Texture2D& texture);
}

//=============================================================================
// VertexBuffer
//=============================================================================

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

//=============================================================================
// IndexBuffer
//=============================================================================

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
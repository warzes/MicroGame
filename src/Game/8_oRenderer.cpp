#include "stdafx.h"
#include "3_Core.h"
#include "6_Platform.h"
#include "8_oRenderer.h"

#include <stb/stb_image.h>

extern glm::vec3 ClearColor;
extern float perspectiveFOV;
extern float perspectiveNear;
extern float perspectiveFar;
extern glm::mat4 projectionMatrix;
namespace
{
	unsigned currentTexture2D[8] = { 0 };
	unsigned currentVAO = 0;
	FrameBuffer* currentFrameBuffer = nullptr;
}

//=============================================================================
// Vertex Attributes
//=============================================================================

constexpr const uint8_t s_attribTypeSizeD3D1x[static_cast<size_t>(VertexAttributeType::Count)][4] =
{
	{  1,  2,  4,  4 }, // Uint8
	{  4,  4,  4,  4 }, // Uint10
	{  2,  4,  8,  8 }, // Int16
	{  2,  4,  8,  8 }, // Half
	{  4,  8, 12, 16 }, // Float
};

constexpr const uint8_t s_attribTypeSizeGl[static_cast<size_t>(VertexAttributeType::Count)][4] =
{
	{  1,  2,  4,  4 }, // Uint8
	{  4,  4,  4,  4 }, // Uint10
	{  2,  4,  6,  8 }, // Int16
	{  2,  4,  6,  8 }, // Half
	{  4,  8, 12, 16 }, // Float
};

constexpr const uint8_t(*s_attribTypeSize)[static_cast<size_t>(VertexAttributeType::Count)][4] =
{
	//&s_attribTypeSizeD3D1x, // Direct3D11/Direct3D12/Vulkan/WebGPU
	&s_attribTypeSizeGl, // OpenGLES/OpenGL
};
//-----------------------------------------------------------------------------
VertexLayout& VertexLayout::Begin()
{
	m_hash = 0;
	m_stride = 0;
	memset(m_attributes, 0xff, sizeof(m_attributes));
	memset(m_offset, 0, sizeof(m_offset));
	return *this;
}
//-----------------------------------------------------------------------------
void VertexLayout::End()
{
	HashMurmur2A murmur;
	murmur.Begin();
	murmur.Add(m_attributes, sizeof(m_attributes));
	murmur.Add(m_offset, sizeof(m_offset));
	murmur.Add(m_stride);
	m_hash = murmur.End();
}
//-----------------------------------------------------------------------------
VertexLayout& VertexLayout::Add(VertexAttribute attrib, uint8_t num, VertexAttributeType type, bool normalized, bool asInt)
{
	const uint16_t encodedNorm = (normalized & 1) << 7;
	const uint16_t encodedType = (static_cast<size_t>(type) & 7) << 3;
	const uint16_t encodedNum = (num - 1) & 3;
	const uint16_t encodeAsInt = (asInt & (!!"\x1\x1\x1\x0\x0"[static_cast<size_t>(type)])) << 8;
	m_attributes[static_cast<size_t>(attrib)] = encodedNorm | encodedType | encodedNum | encodeAsInt;

	m_offset[static_cast<size_t>(attrib)] = m_stride;
	m_stride += (*s_attribTypeSize)[static_cast<size_t>(type)][num - 1];

	return *this;
}
//-----------------------------------------------------------------------------
VertexLayout& VertexLayout::Skip(uint8_t num)
{
	m_stride +=num;
	return *this;
}
//-----------------------------------------------------------------------------
void VertexLayout::Decode(VertexAttribute attrib, uint8_t& num, VertexAttributeType& type, bool& normalized, bool& asInt) const
{
	uint16_t val = m_attributes[static_cast<size_t>(attrib)];
	num = (val & 3) + 1;
	type = static_cast<VertexAttributeType>((val >> 3) & 7);
	normalized = !!(val & (1 << 7));
	asInt = !!(val & (1 << 8));
}
//-----------------------------------------------------------------------------




inline GLenum translate(RenderResourceUsage usage)
{
	switch (usage)
	{
	case RenderResourceUsage::Static:  return GL_STATIC_DRAW;
	case RenderResourceUsage::Dynamic: return GL_DYNAMIC_DRAW;
	case RenderResourceUsage::Stream:  return GL_STREAM_DRAW;
	}
	return 0;
}



inline GLint translate(TextureWrapping wrap)
{
	switch (wrap)
	{
	case TextureWrapping::Repeat:         return GL_REPEAT;
	case TextureWrapping::MirroredRepeat: return GL_MIRRORED_REPEAT;
	case TextureWrapping::Clamp:          return GL_CLAMP_TO_EDGE;
	}
	return 0;
}

inline GLint translate(TextureMinFilter filter)
{
	switch (filter)
	{
	case TextureMinFilter::Nearest:              return GL_NEAREST;
	case TextureMinFilter::Linear:               return GL_LINEAR;
	case TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
	case TextureMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
	case TextureMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
	case TextureMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
	}
	return 0;
}

inline GLint translate(TextureMagFilter filter)
{
	switch (filter)
	{
	case TextureMagFilter::Nearest: return GL_NEAREST;
	case TextureMagFilter::Linear:  return GL_LINEAR;
	}
	return 0;
}

inline bool getTextureFormatType(TexelsFormat inFormat, GLenum textureType, GLenum& format, GLint& internalFormat, GLenum& oglType)
{
	if (inFormat == TexelsFormat::R_U8)
	{
		format = GL_RED;
		internalFormat = GL_R8;
		oglType = GL_UNSIGNED_BYTE;
	}
	else if (inFormat == TexelsFormat::RG_U8)
	{
		format = GL_RG;
		internalFormat = GL_RG8;
		oglType = GL_UNSIGNED_BYTE;
		const GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
		glTexParameteriv(textureType, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask); // TODO: ����� ���� �������� � ����������, ����� ������ ������� stb � ��������� ������� �������
	}
	else if (inFormat == TexelsFormat::RGB_U8)
	{
		format = GL_RGB;
		internalFormat = GL_RGB8;
		oglType = GL_UNSIGNED_BYTE;
	}
	else if (inFormat == TexelsFormat::RGBA_U8)
	{
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
		oglType = GL_UNSIGNED_BYTE;
	}
	else if (inFormat == TexelsFormat::Depth_U16)
	{
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT16;
		oglType = GL_UNSIGNED_SHORT;
	}
	else if (inFormat == TexelsFormat::DepthStencil_U16)
	{
		format = GL_DEPTH_STENCIL;
		internalFormat = GL_DEPTH24_STENCIL8;
		oglType = GL_UNSIGNED_SHORT;
	}
	else if (inFormat == TexelsFormat::Depth_U24)
	{
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT24;
		oglType = GL_UNSIGNED_INT;
	}
	else if (inFormat == TexelsFormat::DepthStencil_U24)
	{
		format = GL_DEPTH_STENCIL;
		internalFormat = GL_DEPTH24_STENCIL8;
		oglType = GL_UNSIGNED_INT;
	}
	else
	{
		LogError("unknown texture format");
		return false;
	}
	return true;
}

bool Texture2D::CreateFromMemories(const Texture2DCreateInfo& createInfo)
{
	if (id > 0) Destroy();

	isTransparent = createInfo.isTransparent;

	// save prev pixel store state
	GLint Alignment = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &Alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// gen texture res
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, translate(createInfo.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, translate(createInfo.wrapT));

	// set texture filtering parameters
	TextureMinFilter minFilter = createInfo.minFilter;
	if (!createInfo.mipmap)
	{
		if (createInfo.minFilter == TextureMinFilter::NearestMipmapNearest) minFilter = TextureMinFilter::Nearest;
		else if (createInfo.minFilter != TextureMinFilter::Nearest) minFilter = TextureMinFilter::Linear;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translate(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translate(createInfo.magFilter));

	// set texture format
	GLenum format = GL_RGB;
	GLint internalFormat = GL_RGB;
	GLenum oglType = GL_UNSIGNED_BYTE;
	getTextureFormatType(createInfo.format, GL_TEXTURE_2D, format, internalFormat, oglType);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, createInfo.width, createInfo.height, 0, format, oglType, createInfo.data);
	if (createInfo.mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	// restore prev state
	glBindTexture(GL_TEXTURE_2D, currentTexture2D[0]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);
	return true;
}

bool Texture2D::CreateFromFiles(const Texture2DLoaderInfo& loaderInfo)
{
	if (loaderInfo.fileName == nullptr || loaderInfo.fileName == "") return false;

	Texture2DCreateInfo createInfo(loaderInfo);
	{
		if (loaderInfo.verticallyFlip)
			stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

		int width = 0;
		int height = 0;
		int nrChannels = 0;
		TexelsFormat format = TexelsFormat::RGB_U8;
		uint8_t* data = stbi_load(loaderInfo.fileName, &width, &height, &nrChannels, 0);
		if (!data || nrChannels < STBI_grey || nrChannels > STBI_rgb_alpha || width == 0 || height == 0)
		{
			LogError("Texture loading failed! Filename='" + std::string(loaderInfo.fileName) + "'");
			return false;
		}
		if (nrChannels == STBI_grey) format = TexelsFormat::R_U8;
		else if (nrChannels == STBI_grey_alpha) format = TexelsFormat::RG_U8;
		else if (nrChannels == STBI_rgb) format = TexelsFormat::RGB_U8;
		else if (nrChannels == STBI_rgb_alpha) format = TexelsFormat::RGBA_U8;

		// ��������� �� ������������
		// TODO: ����� ���� ��������, ��������� �������� � �������� ������ �������
		createInfo.isTransparent = false;
		if (format == TexelsFormat::RGBA_U8)
		{
			for (int i = 0; i < width * height * nrChannels; i += 4)
			{
				//uint8_t r = data[i];
				//uint8_t g = data[i + 1];
				//uint8_t b = data[i + 2];
				const uint8_t& a = data[i + 3];
				if (a < 255)
				{
					createInfo.isTransparent = true;
					break;
				}
			}
		}

		createInfo.format = format;
		createInfo.width = static_cast<uint16_t>(width);
		createInfo.height = static_cast<uint16_t>(height);
		createInfo.depth = 1;
		createInfo.data = data;

		bool isValid = CreateFromMemories(createInfo);
		stbi_image_free((void*)data);
		if (!isValid) return false;
	}

	return true;
}

void Texture2D::Destroy()
{
	if (id > 0)
	{
		for (unsigned i = 0; i < 8; i++)
		{
			if (currentTexture2D[i] == id)
				Texture2D::UnBind(i);
		}
		glDeleteTextures(1, &id);
		id = 0;
	}
}

void Texture2D::Bind(unsigned slot) const
{
	if (currentTexture2D[slot] != id)
	{
		currentTexture2D[slot] = id;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
	}
}

void Texture2D::UnBind(unsigned slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
	currentTexture2D[slot] = 0;
}

bool VertexBuffer::Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data)
{
	if (m_id > 0) Destroy();

	m_vertexCount = vertexCount;
	m_vertexSize = vertexSize;
	m_usage = usage;
	glGenBuffers(1, &m_id);

	GLint currentVBO = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * m_vertexSize, data, translate(m_usage));

	glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(currentVBO));
	return true;
}

void VertexBuffer::Destroy()
{
	GLint currentVBO = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);
	if (static_cast<GLuint>(currentVBO) == m_id) glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (m_id) glDeleteBuffers(1, &m_id);
	m_id = 0;
}

void VertexBuffer::Update(unsigned offset, unsigned size, const void* data)
{
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

bool IndexBuffer::Create(RenderResourceUsage usage, unsigned indexCount, unsigned indexSize, const void* data)
{
	if (m_id > 0) Destroy();

	m_indexCount = indexCount;
	m_indexSize = indexSize;
	m_usage = usage;
	glGenBuffers(1, &m_id);

	GLint currentIBO = 0;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, data, translate(m_usage));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(currentIBO));
	return true;
}

void IndexBuffer::Destroy()
{
	glDeleteBuffers(1, &m_id);
	m_id = 0;
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

inline GLenum translate(PrimitiveDraw p)
{
	switch (p)
	{
	case PrimitiveDraw::Lines:     return GL_LINES;
	case PrimitiveDraw::Triangles: return GL_TRIANGLES;
	case PrimitiveDraw::Points:    return GL_POINTS;
	}
	return 0;
}

inline GLenum translate(VertexAttributeTypeRaw p)
{
	switch (p)
	{
	case VertexAttributeTypeRaw::Float:     return GL_FLOAT;
	}
	return 0;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_id > 0) Destroy();
	if (!vbo || attribs.empty()) return false;

	m_ibo = ibo;
	m_vbo = vbo;

	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);

	vbo->Bind();
	for (size_t i = 0; i < attribs.size(); i++)
	{
		const auto& att = attribs[i];
		glEnableVertexAttribArray(static_cast<GLuint>(i));
		glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
	}
	m_attribsCount = attribs.size();

	if (m_ibo) m_ibo->Bind();

	glBindVertexArray(currentVAO);
	return true;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs, const std::vector<VertexAttributeRaw>& instanceAttribs)
{
	if (!Create(vbo, ibo, attribs))
		return false;

	SetInstancedBuffer(instanceBuffer, instanceAttribs);
	return true;
}

bool VertexArrayBuffer::Create(const std::vector<VertexBuffer*>& vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_id > 0) Destroy();
	if (vbo.size() != attribs.size()) return false;

	m_ibo = ibo;
	m_vbo = vbo[0];

	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);

	for (size_t i = 0; i < vbo.size(); i++)
	{
		const auto& att = attribs[i];
		glEnableVertexAttribArray(static_cast<GLuint>(i));
		vbo[i]->Bind();
		glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, att.stride, att.pointer);
	}
	if (m_ibo) m_ibo->Bind();

	glBindVertexArray(currentVAO);
	return true;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, ShaderProgram* shaders)
{
	if (!shaders || !shaders->IsValid()) return false;

	auto attribInfo = shaders->GetAttribInfo();
	if (attribInfo.empty()) return false;

	size_t stride = 0;
	size_t offset = 0;
	std::vector<VertexAttributeRaw> attribs(attribInfo.size());
	for (int i = 0; i < attribInfo.size(); i++)
	{
		if (attribInfo[i].location != i)
		{
			// TODO: ������� ����������� �������� ������� ���������. ������ ��������������� ��� ������ ������� - 0, ������ - 1 � � �������
			LogError("Shader attribute location: " + std::to_string(attribInfo[i].location) + " not support!");
			return false;
		}

		size_t sizeType = 0;

		attribs[i].normalized = false;
		switch (attribInfo[i].typeId)
		{
		case GL_FLOAT:
			attribs[i].size = 1;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC2:
			attribs[i].size = 2;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC3:
			attribs[i].size = 3;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC4:
			attribs[i].size = 4;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;

		default:
			LogError("Shader attribute type: " + attribInfo[i].typeName + " not support!");
			return false;
		}

		attribs[i].pointer = (void*)+offset;
		offset += sizeType;
	}
	for (int i = 0; i < attribs.size(); i++)
	{
		attribs[i].stride = offset;
	}

	return Create(vbo, ibo, attribs);
}

void VertexArrayBuffer::Destroy()
{
	if (m_id > 0 && currentVAO == m_id)
	{
		if (m_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		currentVAO = 0;
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_id);
	}
	m_id = 0;
	m_instanceBuffer = nullptr;
}

void VertexArrayBuffer::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_instanceBuffer == instanceBuffer) return;

	m_instanceBuffer = instanceBuffer;

	while (m_instancedAttribsCount > m_attribsCount)
	{
		glDisableVertexAttribArray(m_instancedAttribsCount);
		m_instancedAttribsCount--;
	}
	m_instancedAttribsCount = m_attribsCount;

#if 0
	// TODO: ������ ��� ��� �������, ���� ���-�� ������� ����� ��� ������ ���� ����
	{
		glBindVertexArray(m_id);
		instanceBuffer->Bind();
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
	}
#else
	{
		glBindVertexArray(m_id);
		instanceBuffer->Bind();

		for (size_t i = 0; i < attribs.size(); i++)
		{
			m_instancedAttribsCount = m_instancedAttribsCount + i;
			const auto& att = attribs[i];

			if (att.type == VertexAttributeTypeRaw::Matrix4)
			{
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)0);
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
			}
			else
			{
				// TODO: ���������
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
			}
		}
	}
#endif

	glBindVertexArray(currentVAO);
}

void VertexArrayBuffer::Draw(PrimitiveDraw primitive, uint32_t instanceCount)
{
	if (instanceCount == 0) return;

	if (currentVAO != m_id)
	{
		currentVAO = m_id;
		glBindVertexArray(m_id);
	}

	if (m_instanceBuffer)
	{
		if (instanceCount == 1 || instanceCount > m_instanceBuffer->GetVertexCount())
			instanceCount = m_instanceBuffer->GetVertexCount();
	}

	if (m_ibo)
	{
		const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

		if (instanceCount > 1)
			glDrawElementsInstanced(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr, instanceCount);
		else
			glDrawElements(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr);

	}
	else
	{
		if (instanceCount > 1)
			glDrawArraysInstanced(translate(primitive), 0, m_vbo->GetVertexCount(), instanceCount);
		else
			glDrawArrays(translate(primitive), 0, m_vbo->GetVertexCount());
	}
}

void VertexArrayBuffer::DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex)
{
	if (!m_ibo) return;

	if (currentVAO != m_id)
	{
		currentVAO = m_id;
		glBindVertexArray(m_id);
	}

	const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElementsBaseVertex(translate(primitive), indexCount, indexSizeType, (void*)(m_ibo->GetIndexSize() * baseIndex), baseVertex);
}

void VertexArrayBuffer::UnBind()
{
	currentVAO = 0;
	glBindVertexArray(0);
}

bool FrameBuffer::Create(int width, int height)
{
	if (width < 1 || height < 1) return false;
	m_width = width;
	m_height = height;
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	glGenTextures(1, &m_texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
#if 1
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#else
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, nullptr);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // TODO: GL_LINEAR 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Texture2D::UnBind(); // TODO:

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texColorBuffer, 0);

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LogError("Framebuffer is not complete!");
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	currentFrameBuffer = nullptr;
	const float aspect = (float)width / (float)height;
	const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / aspect) * 2.0f;
	m_projectionMatrix = glm::perspective(FOVY, aspect, perspectiveNear, perspectiveFar);

	return true;
}

void FrameBuffer::Destroy()
{
	if (currentFrameBuffer == this) MainFrameBufferBind();

	glDeleteTextures(1, &m_texColorBuffer);
	glDeleteRenderbuffers(1, &m_rbo);
	glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::Bind(const glm::vec3& color)
{
	if (currentFrameBuffer != this)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glViewport(0, 0, m_width, m_height);
		currentFrameBuffer = this;
	}
	glClearColor(color.x, color.y, color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::MainFrameBufferBind()
{
	if (!currentFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GetFrameBufferWidth(), GetFrameBufferHeight());
	glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	currentFrameBuffer = nullptr;
}

void FrameBuffer::BindTextureBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
	Texture2D::UnBind(); // TODO:
}

const glm::mat4& GetCurrentProjectionMatrix()
{
	if (currentFrameBuffer)
		return currentFrameBuffer->GetProjectionMatrix();
	else
		return projectionMatrix;
}

namespace TextureFileManager
{
	std::unordered_map<std::string, Texture2D> FileTextures;

	void Destroy()
	{
		for (auto it = FileTextures.begin(); it != FileTextures.end(); ++it)
			it->second.Destroy();
		FileTextures.clear();
	}

	Texture2D* LoadTexture2D(const char* name)
	{
		Texture2DLoaderInfo textureLoaderInfo = {};
		textureLoaderInfo.fileName = name;
		return LoadTexture2D(textureLoaderInfo);
	}

	Texture2D* LoadTexture2D(const Texture2DLoaderInfo& textureLoaderInfo)
	{
		auto it = FileTextures.find(textureLoaderInfo.fileName);
		if (it != FileTextures.end())
		{
			return &it->second;
		}
		else
		{
			LogPrint("Load texture: " + std::string(textureLoaderInfo.fileName));

			Texture2D texture;
			if (!texture.CreateFromFiles(textureLoaderInfo) || !texture.IsValid())
				return nullptr;

			FileTextures[textureLoaderInfo.fileName] = texture;
			return &FileTextures[textureLoaderInfo.fileName];
		}
	}
}
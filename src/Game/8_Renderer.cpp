#include "stdafx.h"
#include "3_Core.h"
#include "6_Platform.h"
#include "8_Renderer.h"

#include <stb/stb_image.h>

namespace
{
	int RenderWidth = 0;
	int RenderHeight = 0;
	glm::vec3 ClearColor;

	unsigned currentShaderProgram = 0;
	unsigned currentTexture2D[8] = { 0 };
	unsigned currentVAO = 0;
	FrameBuffer* currentFrameBuffer = nullptr;

	glm::mat4 projectionMatrix;
	float perspectiveFOV = 0.0f;
	float perspectiveNear = 0.01f;
	float perspectiveFar = 1000.0f;
}

#if defined(_DEBUG)
void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept
{
	// Ignore non-significant error/warning codes (NVidia drivers)
	// NOTE: Here there are the details with a sample output:
	// - #131169 - Framebuffer detailed info: The driver allocated storage for renderbuffer 2. (severity: low)
	// - #131185 - Buffer detailed info: Buffer object 1 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_ENUM_88e4)
	//             will use VIDEO memory as the source for buffer object operations. (severity: low)
	// - #131218 - Program/shader state performance warning: Vertex shader in program 7 is being recompiled based on GL state. (severity: medium)
	// - #131204 - Texture state usage warning: The texture object (0) bound to texture image unit 0 does not have
	//             a defined base level and cannot be used for texture mapping. (severity: low)
	if ((id == 131169) || (id == 131185) || (id == 131218) || (id == 131204)) return;

	const char* msgSource = nullptr;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API: msgSource = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: msgSource = "WINDOW_SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: msgSource = "SHADER_COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: msgSource = "THIRD_PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: msgSource = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER: msgSource = "OTHER"; break;
	default: break;
	}

	const char* msgType = nullptr;
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: msgType = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msgType = "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: msgType = "UNDEFINED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY: msgType = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: msgType = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_MARKER: msgType = "MARKER"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: msgType = "PUSH_GROUP"; break;
	case GL_DEBUG_TYPE_POP_GROUP: msgType = "POP_GROUP"; break;
	case GL_DEBUG_TYPE_OTHER: msgType = "OTHER"; break;
	default: break;
	}

	const char* msgSeverity = "DEFAULT";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW: msgSeverity = "LOW"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: msgSeverity = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_HIGH: msgSeverity = "HIGH"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: msgSeverity = "NOTIFICATION"; break;
	default: break;
	}

	LogError("GL: OpenGL debug message: " + std::string(message));
	LogError("    > Type: " + std::string(msgType));
	LogError("    > Source = " + std::string(msgSource));
	LogError("    > Severity = " + std::string(msgSeverity));
}
#endif

bool CreateRenderSystem(const RendererCreateInfo& createInfo)
{
#if defined(_DEBUG)
	if ((glDebugMessageCallback != NULL) && (glDebugMessageControl != NULL))
	{
		glDebugMessageCallback(debugMessageCallback, 0);
		// glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, 0, GL_TRUE); // TODO: Filter message

		// Debug context options:
		//  - GL_DEBUG_OUTPUT - Faster version but not useful for breakpoints
		//  - GL_DEBUG_OUTPUT_SYNCHRONUS - Callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	ClearColor = createInfo.ClearColor;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
	glClearDepth(1.0f);
	glViewport(0, 0, GetFrameBufferWidth(), GetFrameBufferHeight());

	perspectiveFOV = createInfo.PerspectiveFOV;
	perspectiveNear = createInfo.PerspectiveNear;
	perspectiveFar = createInfo.PerspectiveFar;

	const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / GetFrameBufferAspectRatio()) * 2.0f;
	projectionMatrix = glm::perspective(FOVY, GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);

	return true;
}

void DestroyRenderSystem()
{

}

void BeginRenderFrame()
{
	if (RenderWidth != GetFrameBufferWidth() || RenderHeight != GetFrameBufferHeight())
	{
		RenderWidth = GetFrameBufferWidth();
		RenderHeight = GetFrameBufferHeight();
		glViewport(0, 0, RenderWidth, RenderHeight);
		const float FOVY2 = glm::radians(perspectiveFOV);
		const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / GetFrameBufferAspectRatio()) * 2.0f;
		projectionMatrix = glm::perspective(FOVY, GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);

	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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

bool ShaderProgram::CreateFromMemories(const char* vertexShaderMemory, const char* fragmentShaderMemory)
{
	if (vertexShaderMemory == nullptr || fragmentShaderMemory == nullptr) return false;
	if (vertexShaderMemory == "" || fragmentShaderMemory == "") return false;
	if (m_id > 0) Destroy();

	const GLuint shaderVertex = createShader(GL_VERTEX_SHADER, vertexShaderMemory);
	const GLuint shaderFragment = createShader(GL_FRAGMENT_SHADER, fragmentShaderMemory);

	if (shaderVertex > 0 && shaderFragment > 0)
	{
		m_id = glCreateProgram();
		glAttachShader(m_id, shaderVertex);
		glAttachShader(m_id, shaderFragment);

		glLinkProgram(m_id);
		GLint success = 0;
		glGetProgramiv(m_id, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint errorMsgLen;
			glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &errorMsgLen);

			std::vector<GLchar> errorInfo(errorMsgLen);
			glGetProgramInfoLog(m_id, errorInfo.size(), nullptr, &errorInfo[0]);
			LogError("OPENGL: Shader program linking failed: " + std::string(&errorInfo[0]));
			glDeleteProgram(m_id);
			m_id = 0;
		}
	}

	glDeleteShader(shaderVertex);
	glDeleteShader(shaderFragment);

	return IsValid();
}

void ShaderProgram::Destroy()
{
	if (m_id > 0)
	{
		if (currentShaderProgram == m_id) UnBind();
		glDeleteProgram(m_id);
		m_id = 0;
	}
}

void ShaderProgram::Bind()
{
	if (currentShaderProgram != m_id)
	{
		currentShaderProgram = m_id;
		glUseProgram(currentShaderProgram);
	}
}

void ShaderProgram::UnBind()
{
	currentShaderProgram = 0;
	glUseProgram(0);
}

UniformVariable ShaderProgram::GetUniformVariable(const char* name)
{
	return { glGetUniformLocation(m_id, name) };
}

void ShaderProgram::SetUniform(UniformVariable var, int value)
{
	glUniform1i(var.id, value);
}

void ShaderProgram::SetUniform(UniformVariable var, float value)
{
	glUniform1f(var.id, value);
}

void ShaderProgram::SetUniform(UniformVariable var, const glm::vec2& v)
{
	glUniform2fv(var.id, 1, glm::value_ptr(v));
}

void ShaderProgram::SetUniform(UniformVariable var, float x, float y, float z)
{
	glUniform3f(var.id, x, y, z);
}

void ShaderProgram::SetUniform(UniformVariable var, const glm::vec3& v)
{
	glUniform3fv(var.id, 1, glm::value_ptr(v));
}

void ShaderProgram::SetUniform(UniformVariable var, const glm::vec4& v)
{
	glUniform4fv(var.id, 1, glm::value_ptr(v));
}

void ShaderProgram::SetUniform(UniformVariable var, const glm::mat3& mat)
{
	glUniformMatrix3fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::SetUniform(UniformVariable var, const glm::mat4& mat)
{
	glUniformMatrix4fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::SetUniform(const char* name, int value)
{
	SetUniform(GetUniformVariable(name), value);
}
void ShaderProgram::SetUniform(const char* name, float value)
{
	SetUniform(GetUniformVariable(name), value);
}
void ShaderProgram::SetUniform(const char* name, float x, float y, float z)
{
	SetUniform(GetUniformVariable(name), x, y, z);
}
void ShaderProgram::SetUniform(const char* name, const glm::vec2& v)
{
	SetUniform(GetUniformVariable(name), v);
}
void ShaderProgram::SetUniform(const char* name, const glm::vec3& v)
{
	SetUniform(GetUniformVariable(name), v);
}
void ShaderProgram::SetUniform(const char* name, const glm::vec4& v)
{
	SetUniform(GetUniformVariable(name), v);
}
void ShaderProgram::SetUniform(const char* name, const glm::mat3& mat)
{
	SetUniform(GetUniformVariable(name), mat);
}
void ShaderProgram::SetUniform(const char* name, const glm::mat4& mat)
{
	SetUniform(GetUniformVariable(name), mat);
}

unsigned ShaderProgram::createShader(unsigned type, const char* shaderString) const
{
	if (shaderString == nullptr || shaderString == "") return 0;

	const GLuint shaderId = glCreateShader(type);
	glShaderSource(shaderId, 1, &shaderString, nullptr);
	glCompileShader(shaderId);

	GLint success = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint infoLogSize;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogSize);
		std::vector<GLchar> errorInfo(infoLogSize);
		glGetShaderInfoLog(shaderId, errorInfo.size(), nullptr, &errorInfo[0]);
		glDeleteShader(shaderId);
		const std::string msg = "Shader compilation failed : " + std::string(&errorInfo[0]) + ", Source: " + shaderString;
		LogError(msg);
		return 0;
	}

	return shaderId;
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
		glTexParameteriv(textureType, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask); // TODO: могут быть проблемы с браузерами, тогда только грузить stb с указанием нужного формата
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

		// проверить на прозрачность
		// TODO: может быть медленно, проверить скорость и поискать другое решение
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

inline GLenum translate(VertexAttributeType p)
{
	switch (p)
	{
	case VertexAttributeType::Float:     return GL_FLOAT;
	}
	return 0;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs)
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

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs, const std::vector<VertexAttribute>& instanceAttribs)
{
	if (!Create(vbo, ibo, attribs))
		return false;

	SetInstancedBuffer(instanceBuffer, instanceAttribs);
	return true;
}

bool VertexArrayBuffer::Create(const std::vector<VertexBuffer*> vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs)
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
		glEnableVertexAttribArray(i);
		vbo[i]->Bind();
		glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, att.stride, att.pointer);
	}
	if (m_ibo) m_ibo->Bind();

	glBindVertexArray(currentVAO);
	return true;
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

void VertexArrayBuffer::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs)
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
	// TODO: сейчас это под матрицы, надо как-то сделать чтобы под другие типы тоже
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

			if (att.type == VertexAttributeType::Matrix4)
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
				// TODO: проверить
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
﻿#include "stdafx.h"
#include "Base.h"
#include "Core.h"
#include "Renderer.h"
#include "Window.h"
//-----------------------------------------------------------------------------
#if defined(_WIN32)
extern "C"
{
	// NVIDIA: Force usage of NVidia GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> See "Enabling High Performance Graphics Rendering on Optimus Systems" http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

	// AMD: Force usage of AMD GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> Named "Dynamic Switchable Graphics", found no official documentation, only https://community.amd.com/message/1307599#comment-1307599 - "Can an OpenGL app default to the discrete GPU on an Enduro system?"
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // _WIN32
//-----------------------------------------------------------------------------
//=============================================================================
// OpenGL Debug
//=============================================================================
//-----------------------------------------------------------------------------
#if defined(_DEBUG)
inline void openglCheckError(const char* func, const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "GL_INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "GL_INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "GL_INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "GL_STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "GL_STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "GL_OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
		default:                               error = "UNKNOWN"; break;
		}
		LogError("OpenGL Error detected (" + error + ") in func: " + std::string(func) + " - " + std::string(file) + " (" + std::to_string(line) + ")");
	}
}
#endif
#if defined(_DEBUG)
#	define GL_CHECK(_func)                                        \
				{                                                 \
					_func;                                        \
					openglCheckError(#_func, __FILE__, __LINE__); \
				}
#else
#	define GL_CHECK(_func) {_func;}
#endif
//-----------------------------------------------------------------------------
#if defined(_DEBUG)
void openglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) noexcept
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

	std::string msgSource;
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

	std::string msgType;
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

	std::string msgSeverity = "DEFAULT";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW: msgSeverity = "LOW"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: msgSeverity = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_HIGH: msgSeverity = "HIGH"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: msgSeverity = "NOTIFICATION"; break;
	default: break;
	}

	LogError("GL: OpenGL debug message: " + std::string(message));
	LogError("    > Type: " + msgType);
	LogError("    > Source: " + msgSource);
	LogError("    > Severity: " + msgSeverity);
}
#endif
//-----------------------------------------------------------------------------
//=============================================================================
// OpenGL Core func
//=============================================================================
//-----------------------------------------------------------------------------
inline constexpr GLsizei GetFormatComponentCount(const GLenum format)
{
	switch (format)
	{
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_RED_INTEGER:
	case GL_GREEN:
	case GL_GREEN_INTEGER:
	case GL_BLUE:
	case GL_BLUE_INTEGER:
		return 1;

	case GL_DEPTH_STENCIL:
	case GL_RG:
	case GL_RG_INTEGER:
		return 2;

	case GL_RGB:
	case GL_RGB_INTEGER:
	case GL_BGR:
	case GL_BGR_INTEGER:
		return 3;

	case GL_RGBA:
	case GL_RGBA_INTEGER:
	case GL_BGRA:
	case GL_BGRA_INTEGER:
		return 4;

	default:
		assert("Unsupported OpenGL format!");
		return 0;
	}
}
//-----------------------------------------------------------------------------
inline constexpr GLsizei GetTypeSize(const GLenum type)
{
	switch (type)
	{
	case GL_UNSIGNED_BYTE:
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		return sizeof(GLubyte);

	case GL_BYTE:
		return sizeof(GLbyte);

	case GL_UNSIGNED_SHORT:
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		return sizeof(GLushort);

	case GL_SHORT:
		return sizeof(GLshort);

	case GL_UNSIGNED_INT:
	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_24_8:
	case GL_UNSIGNED_INT_10F_11F_11F_REV:
	case GL_UNSIGNED_INT_5_9_9_9_REV:
		return sizeof(GLuint);

	case GL_INT:
		return sizeof(GLint);

	case GL_HALF_FLOAT:
		return sizeof(GLhalf);

	case GL_FLOAT:
		return sizeof(GLfloat);

	default:
		assert("Unsupported OpenGL type!");
		return 0;
	}
}
//-----------------------------------------------------------------------------
inline const char* glslTypeName(GLuint type)
{
#define GLSL_TYPE(_ty) case _ty: return #_ty
	switch (type)
	{
		GLSL_TYPE(GL_BOOL);
		GLSL_TYPE(GL_INT);
		GLSL_TYPE(GL_INT_VEC2);
		GLSL_TYPE(GL_INT_VEC3);
		GLSL_TYPE(GL_INT_VEC4);
		GLSL_TYPE(GL_UNSIGNED_INT);
		GLSL_TYPE(GL_UNSIGNED_INT_VEC2);
		GLSL_TYPE(GL_UNSIGNED_INT_VEC3);
		GLSL_TYPE(GL_UNSIGNED_INT_VEC4);
		GLSL_TYPE(GL_FLOAT);
		GLSL_TYPE(GL_FLOAT_VEC2);
		GLSL_TYPE(GL_FLOAT_VEC3);
		GLSL_TYPE(GL_FLOAT_VEC4);
		GLSL_TYPE(GL_FLOAT_MAT2);
		GLSL_TYPE(GL_FLOAT_MAT3);
		GLSL_TYPE(GL_FLOAT_MAT4);

		GLSL_TYPE(GL_SAMPLER_2D);
		GLSL_TYPE(GL_SAMPLER_2D_ARRAY);
		GLSL_TYPE(GL_SAMPLER_2D_MULTISAMPLE);

		GLSL_TYPE(GL_INT_SAMPLER_2D);
		GLSL_TYPE(GL_INT_SAMPLER_2D_ARRAY);
		GLSL_TYPE(GL_INT_SAMPLER_2D_MULTISAMPLE);

		GLSL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D);
		GLSL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
		GLSL_TYPE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);

		GLSL_TYPE(GL_SAMPLER_2D_SHADOW);
		GLSL_TYPE(GL_SAMPLER_2D_ARRAY_SHADOW);

		GLSL_TYPE(GL_SAMPLER_3D);
		GLSL_TYPE(GL_INT_SAMPLER_3D);
		GLSL_TYPE(GL_UNSIGNED_INT_SAMPLER_3D);

		GLSL_TYPE(GL_SAMPLER_CUBE);
		GLSL_TYPE(GL_INT_SAMPLER_CUBE);
		GLSL_TYPE(GL_UNSIGNED_INT_SAMPLER_CUBE);

		GLSL_TYPE(GL_IMAGE_1D);
		GLSL_TYPE(GL_INT_IMAGE_1D);
		GLSL_TYPE(GL_UNSIGNED_INT_IMAGE_1D);

		GLSL_TYPE(GL_IMAGE_2D);
		GLSL_TYPE(GL_IMAGE_2D_ARRAY);
		GLSL_TYPE(GL_INT_IMAGE_2D);
		GLSL_TYPE(GL_UNSIGNED_INT_IMAGE_2D);

		GLSL_TYPE(GL_IMAGE_3D);
		GLSL_TYPE(GL_INT_IMAGE_3D);
		GLSL_TYPE(GL_UNSIGNED_INT_IMAGE_3D);

		GLSL_TYPE(GL_IMAGE_CUBE);
		GLSL_TYPE(GL_INT_IMAGE_CUBE);
		GLSL_TYPE(GL_UNSIGNED_INT_IMAGE_CUBE);
	}
#undef GLSL_TYPE
	return "UNKNOWN GLSL TYPE!";
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//=============================================================================
// Type translation
//=============================================================================
//-----------------------------------------------------------------------------
inline constexpr GLenum translate(RenderResourceUsage usage)
{
	switch (usage)
	{
	case RenderResourceUsage::Static:  return GL_STATIC_DRAW;
	case RenderResourceUsage::Dynamic: return GL_DYNAMIC_DRAW;
	case RenderResourceUsage::Stream:  return GL_STREAM_DRAW;
	}
	return 0;
}
//-----------------------------------------------------------------------------
inline constexpr GLenum translate(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:    return GL_VERTEX_SHADER;
	case ShaderType::Geometry:  return GL_GEOMETRY_SHADER;
	case ShaderType::Fragment:  return GL_FRAGMENT_SHADER;
	}
	return 0;
}
//-----------------------------------------------------------------------------
inline constexpr GLint translate(TextureWrapping wrap)
{
	switch (wrap)
	{
	case TextureWrapping::Repeat:         return GL_REPEAT;
	case TextureWrapping::MirroredRepeat: return GL_MIRRORED_REPEAT;
	case TextureWrapping::Clamp:          return GL_CLAMP_TO_EDGE;
	}
	return 0;
}
//-----------------------------------------------------------------------------
inline constexpr GLint translate(TextureMinFilter filter)
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
//-----------------------------------------------------------------------------
inline constexpr GLint translate(TextureMagFilter filter)
{
	switch (filter)
	{
	case TextureMagFilter::Nearest: return GL_NEAREST;
	case TextureMagFilter::Linear:  return GL_LINEAR;
	}
	return 0;
}
//-----------------------------------------------------------------------------
inline constexpr GLenum translate(PrimitiveDraw p)
{
	switch (p)
	{
	case PrimitiveDraw::Lines:     return GL_LINES;
	case PrimitiveDraw::Triangles: return GL_TRIANGLES;
	case PrimitiveDraw::Points:    return GL_POINTS;
	}
	return 0;
}
//-----------------------------------------------------------------------------
inline constexpr GLenum translate(VertexAttributeTypeRaw p)
{
	switch (p)
	{
	case VertexAttributeTypeRaw::Float:     return GL_FLOAT;
	}
	return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
// Renderer State
//=============================================================================
//-----------------------------------------------------------------------------
namespace RendererState
{
#if USE_OPENGL_CACHE_STATE
	unsigned currentShaderProgram = 0;
	unsigned currentTexture2D[MAXTEXTURE] = { 0 };

	unsigned currentVAO = 0;

	FrameBuffer* currentFrameBuffer = nullptr;

	// Blending variables
	//int currentBlendMode;               // Blending mode active
	//int glBlendSrcFactor;               // Blending source factor
	//int glBlendDstFactor;               // Blending destination factor
	//int glBlendEquation;                // Blending equation
	//int glBlendSrcFactorRGB;            // Blending source RGB factor
	//int glBlendDestFactorRGB;           // Blending destination RGB factor
	//int glBlendSrcFactorAlpha;          // Blending source alpha factor
	//int glBlendDestFactorAlpha;         // Blending destination alpha factor
	//int glBlendEquationRGB;             // Blending equation for RGB
	//int glBlendEquationAlpha;           // Blending equation for alpha
	//bool glCustomBlendModeModified;     // Custom blending factor and equation modification status
#endif

	int framebufferWidth = 0;
	int framebufferHeight = 0;

	glm::vec3 ClearColor;
	float perspectiveFOV = 0.0f;
	float perspectiveNear = 0.01f;
	float perspectiveFar = 1000.0f;
	glm::mat4 projectionMatrix;
}
//-----------------------------------------------------------------------------
//=============================================================================
// ShaderProgram
//=============================================================================
//-----------------------------------------------------------------------------
bool ShaderProgram::CreateFromMemories(const std::string& vertexShaderMemory, const std::string& fragmentShaderMemory)
{
	return CreateFromMemories(vertexShaderMemory, "", fragmentShaderMemory);
}
//-----------------------------------------------------------------------------
bool ShaderProgram::CreateFromMemories(const std::string& vertexShaderMemory, const std::string& geometryShaderMemory, const std::string& fragmentShaderMemory)
{
	// в OpenGL 4.1 есть более краткая glCreateShaderProgram - https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShaderProgram.xhtml
	if (vertexShaderMemory.empty() || fragmentShaderMemory.empty()) return false;
	if (vertexShaderMemory == "" || fragmentShaderMemory == "") return false;
	if (m_id > 0) Destroy();

	const GLuint glShaderVertex = createShader(ShaderType::Vertex, vertexShaderMemory);
	const GLuint glShaderFragment = createShader(ShaderType::Fragment, fragmentShaderMemory);
	GLuint glShaderGeometry = 0;
	if (!geometryShaderMemory.empty() && geometryShaderMemory != "")
		glShaderGeometry = createShader(ShaderType::Geometry, geometryShaderMemory);

	if (glShaderVertex > 0 && glShaderFragment > 0)
	{
		m_id = glCreateProgram();

		GL_CHECK(glAttachShader(m_id, glShaderVertex));
		GL_CHECK(glAttachShader(m_id, glShaderFragment));
		if (glShaderGeometry > 0) GL_CHECK(glAttachShader(m_id, glShaderGeometry));

		GL_CHECK(glLinkProgram(m_id));

		GL_CHECK(glDetachShader(m_id, glShaderVertex));
		GL_CHECK(glDetachShader(m_id, glShaderFragment));
		if (glShaderGeometry > 0) GL_CHECK(glDetachShader(m_id, glShaderGeometry));

		GLint success = 0;
		GL_CHECK(glGetProgramiv(m_id, GL_LINK_STATUS, &success));
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

	GL_CHECK(glDeleteShader(glShaderVertex));
	GL_CHECK(glDeleteShader(glShaderFragment));
	if (glShaderGeometry > 0) GL_CHECK(glDeleteShader(glShaderGeometry));

	LogPrint("Program " + std::to_string(m_id));

	// print log attrib info
	{
		auto attribInfo = GetAttribInfo();
		LogPrint("Attributes (" + std::to_string(attribInfo.size()) + "):");
		for (size_t i = 0; i < attribInfo.size(); i++)
		{
			LogPrint("\t" + attribInfo[i].GetText());
		}
	}

	// print log uniform info
	{
		auto uniformInfo = GetUniformInfo();
		LogPrint("Uniforms (" + std::to_string(uniformInfo.size()) + "):");
		for (size_t i = 0; i < uniformInfo.size(); i++)
		{
			LogPrint("\t" + uniformInfo[i].GetText());
		}
	}


	return IsValid();
}
//-----------------------------------------------------------------------------
void ShaderProgram::Destroy()
{
	if (m_id > 0)
	{
#if USE_OPENGL_CACHE_STATE
		if (RendererState::currentShaderProgram == m_id) UnBind();
#endif
		if (!ShaderLoader::IsLoad(*this)) // TODO: не удалять шейдер если он загружен через менеджер, в будущем сделать подсчет ссылок и удалять если нет
			glDeleteProgram(m_id);
		m_id = 0;
	}
}
//-----------------------------------------------------------------------------
void ShaderProgram::Bind()
{
#if USE_OPENGL_CACHE_STATE
	if (RendererState::currentShaderProgram == m_id)
		return;
	RendererState::currentShaderProgram = m_id;
#endif
	glUseProgram(m_id);
}
//-----------------------------------------------------------------------------
void ShaderProgram::UnBind()
{
#if USE_OPENGL_CACHE_STATE
	RendererState::currentShaderProgram = 0;
#endif
	glUseProgram(0);
}
//-----------------------------------------------------------------------------
std::vector<ShaderAttribInfo> ShaderProgram::GetAttribInfo() const
{
	if (!IsValid()) return {};

	GLint activeAttribs = 0;
	GLint max0, max1;

	const bool piqSupported = GLAD_GL_ARB_program_interface_query && GLAD_GL_ARB_shader_storage_buffer_object;

	if (piqSupported)
	{
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &activeAttribs));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &max0));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_UNIFORM, GL_MAX_NAME_LENGTH, &max1));
	}
	else
	{
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &activeAttribs));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max0));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max1));
	}

	uint32_t maxLength = Max(max0, max1);
	char* name = (char*)alloca(maxLength + 1);

	std::vector<ShaderAttribInfo> attribs;
	for (int32_t i = 0; i < activeAttribs; i++)
	{
		GLint size;
		GLenum type = 0;

		if (piqSupported)
		{
			GL_CHECK(glGetProgramResourceName(m_id, GL_PROGRAM_INPUT, i, maxLength + 1, &size, name));
			GLenum typeProp[] = { GL_TYPE };
			GL_CHECK(glGetProgramResourceiv(m_id, GL_PROGRAM_INPUT, i, Countof(typeProp), typeProp, 1, NULL, (GLint*)&type));
		}
		else
		{
			GL_CHECK(glGetActiveAttrib(m_id, i, maxLength + 1, NULL, &size, &type, name));
		}
		ShaderAttribInfo  attrib;
		attrib.typeId = type;
		attrib.typeName = glslTypeName(type);
		attrib.name = name;
		attrib.location = glGetAttribLocation(m_id, name);
		attribs.emplace_back(attrib);
	}

	std::sort(attribs.begin(), attribs.end(), [](const ShaderAttribInfo& a, const ShaderAttribInfo& b) {return a.location < b.location; });

	return attribs;
}
//-----------------------------------------------------------------------------
std::vector<ShaderUniformInfo> ShaderProgram::GetUniformInfo() const
{
	if (!IsValid()) return {};

	GLint activeAttribs = 0;
	GLint activeUniforms = 0;
	GLint activeBuffers = 0;
	GLint max0, max1;

	const bool piqSupported = GLAD_GL_ARB_program_interface_query && GLAD_GL_ARB_shader_storage_buffer_object;

	if (piqSupported)
	{
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &activeAttribs));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &activeUniforms));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES, &activeBuffers));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &max0));
		GL_CHECK(glGetProgramInterfaceiv(m_id, GL_UNIFORM, GL_MAX_NAME_LENGTH, &max1));
	}
	else
	{
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &activeAttribs));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &activeUniforms));

		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max0));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max1));
	}

	uint32_t maxLength = Max(max0, max1);
	char* name = (char*)alloca(maxLength + 1);

	GLint m_sampler[16/*CONFIG_MAX_TEXTURE_SAMPLERS*/];
	uint8_t m_numSamplers;
	m_numSamplers = 0;

	LogPrint("Uniforms (" + std::to_string(activeUniforms) + "):");
	for (int32_t i = 0; i < activeUniforms; ++i)
	{
		struct VariableInfo
		{
			GLenum type;
			GLint  loc;
			GLint  num;
		};
		VariableInfo vi;
		GLenum props[] = { GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };

		GLenum gltype;
		GLint num;
		GLint loc;

		if (piqSupported)
		{
			GL_CHECK(glGetProgramResourceiv(m_id, GL_UNIFORM, i, Countof(props), props, Countof(props), NULL, (GLint*)&vi
			));
			GL_CHECK(glGetProgramResourceName(m_id, GL_UNIFORM, i, maxLength + 1, NULL, name));

			gltype = vi.type;
			loc = vi.loc;
			num = vi.num;
		}
		else
		{
			GL_CHECK(glGetActiveUniform(m_id, i, maxLength + 1, NULL, &num, &gltype, name));
			loc = glGetUniformLocation(m_id, name);
		}

		num = Max(num, 1);

		// array
		int32_t offset = 0;
		/*const bx::StringView array = bx::strFind(name, '[');
		if (!array.isEmpty())
		{
			name[array.getPtr() - name] = '\0';
			BX_TRACE("--- %s", name);
			const bx::StringView end = bx::strFind(array.getPtr() + 1, ']');
			bx::fromString(&offset, bx::StringView(array.getPtr() + 1, end.getPtr()));
		}*/

		switch (gltype)
		{
		case GL_SAMPLER_2D:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_2D_MULTISAMPLE:

		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:

		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:

		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:

		case GL_SAMPLER_3D:
		case GL_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_3D:

		case GL_SAMPLER_CUBE:
		case GL_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:

		case GL_IMAGE_1D:
		case GL_INT_IMAGE_1D:
		case GL_UNSIGNED_INT_IMAGE_1D:

		case GL_IMAGE_2D:
		case GL_INT_IMAGE_2D:
		case GL_UNSIGNED_INT_IMAGE_2D:

		case GL_IMAGE_3D:
		case GL_INT_IMAGE_3D:
		case GL_UNSIGNED_INT_IMAGE_3D:

		case GL_IMAGE_CUBE:
		case GL_INT_IMAGE_CUBE:
		case GL_UNSIGNED_INT_IMAGE_CUBE:
			if (m_numSamplers < Countof(m_sampler))
			{
				LogPrint("Sampler #" + std::to_string(m_numSamplers) + " at location " + std::to_string(loc));
				m_sampler[m_numSamplers] = loc;
				m_numSamplers++;
			}
			else
			{
				LogPrint("Too many samplers (max: " + std::to_string(Countof(m_sampler)) + ")! Sampler at location " + std::to_string(loc));
			}
			break;

		default:
			break;
		}

		LogPrint("\tuniform " + std::string(glslTypeName(gltype)) + " " + std::string(name) + " is at location " + std::to_string(loc) + ", size " + std::to_string(num) + ", offset " + std::to_string(offset));
	}

	return std::vector<ShaderUniformInfo>();
}
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetInterfaceActiveResources(const unsigned interface) const
{
	return getInterfaceParameter(interface, GL_ACTIVE_RESOURCES);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetInterfaceMaxNameLength(const unsigned interface) const
{
	return getInterfaceParameter(interface, GL_MAX_NAME_LENGTH);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetInterfaceMaxActiveVariableCount(const unsigned interface) const
{
	return getInterfaceParameter(interface, GL_MAX_NUM_ACTIVE_VARIABLES);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetInterfaceMaxCompatibleSubroutineCount(const unsigned interface) const
{
	return getInterfaceParameter(interface, GL_MAX_NUM_COMPATIBLE_SUBROUTINES);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
unsigned ShaderProgram::GetResourceIndex(const unsigned interface, const std::string& name) const
{
	return glGetProgramResourceIndex(m_id, interface, name.c_str());
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
std::string ShaderProgram::GetResourceName(const unsigned interface, const unsigned index) const
{
	std::string name;
	GLsizei length;
	name.resize(GetInterfaceMaxNameLength(interface));
	glGetProgramResourceName(m_id, interface, index, static_cast<GLsizei>(name.size()), &length, &name[0]);
	name.resize(length);
	return name;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetResourceLocation(const unsigned interface, const std::string& name) const
{
	return glGetProgramResourceLocation(m_id, interface, name.c_str());
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetResourceLocationIndex(const unsigned interface, const std::string& name) const
{
	return glGetProgramResourceLocationIndex(m_id, interface, name.c_str());
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::GetResourceParameter(const unsigned interface, const unsigned index, unsigned parameter) const
{
	int result;
	glGetProgramResourceiv(m_id, interface, index, 1, &parameter, 1, nullptr, &result);
	return result;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
std::vector<int> ShaderProgram::GetResourceParameters(const unsigned interface, const unsigned index, const std::vector<unsigned>& parameters) const
{
	std::vector<GLint> result(parameters.size());
	glGetProgramResourceiv(m_id, interface, index, static_cast<GLsizei>(parameters.size()), parameters.data(), static_cast<GLsizei>(result.size()), nullptr, result.data());
	return result;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 41
std::vector<GLbyte> ShaderProgram::GetProgramBinary(unsigned format) const
{
	std::vector<GLbyte> result(GetBinaryLength());
	glGetProgramBinary(m_id, static_cast<GLsizei>(result.size()), nullptr, &format, static_cast<void*>(result.data()));
	return result;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 41
void ShaderProgram::SetProgramBinary(const unsigned format, const std::vector<GLbyte>& binary)
{
	glProgramBinary(m_id, format, static_cast<const void*>(binary.data()), static_cast<GLsizei>(binary.size()));
}
#endif
//-----------------------------------------------------------------------------
int ShaderProgram::GetUniformLocation(const char* name) const
{
	return glGetUniformLocation(m_id, name);
}
//-----------------------------------------------------------------------------
UniformLocation ShaderProgram::GetUniformVariable(const char* name) const
{
	return { GetUniformLocation(name)};
}
//-----------------------------------------------------------------------------
std::string ShaderProgram::GetActiveUniformName(const unsigned index) const
{
	// не забыть что uniform location это не тоже самое что uniform index, поэтому нельзя брать id из UniformLocation
	// получить индексы можно через GetUniformIndices
	std::string result;
	result.resize(GetActiveUniformNameLength(index));
	GL_CHECK(glGetActiveUniformName(m_id, index, static_cast<GLsizei>(result.size()), nullptr, &result[0]));
	return result;
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetUniformIndex(const char* name) const
{
	unsigned index = 0;
	GL_CHECK(glGetUniformIndices(m_id, 1, &name, &index));
	return index;
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetUniformIndices(const int count, const std::vector<std::string>& names) const
{
	std::vector<unsigned> indices(names.size());
	std::vector<const char*> cnames(names.size());
	std::transform(names.begin(), names.end(), cnames.begin(), [&](const std::string& varying) { return varying.c_str(); });
	GL_CHECK(glGetUniformIndices(m_id, count, cnames.data(), indices.data()));
	return indices;
}
//-----------------------------------------------------------------------------
std::tuple<std::string, unsigned, int> ShaderProgram::GetActiveUniform(const unsigned index) const
{
	std::tuple<std::string, unsigned, int> result;
	auto& name = std::get<0>(result);
	auto& type = std::get<1>(result);
	auto& size = std::get<2>(result);
	name.resize(GetActiveUniformMaxLength());
	GL_CHECK(glGetActiveUniform(m_id, index, static_cast<GLsizei>(name.size()), nullptr, &size, &type, &name[0]));
	return result;
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformNameLength(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_NAME_LENGTH);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformGLType(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_TYPE);
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveUniformsGLTypes(const std::vector<unsigned>& indices) const
{
	auto result = getActiveUniformParameters(indices, GL_UNIFORM_TYPE);
	return std::vector<unsigned>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformOffset(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_OFFSET);
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveUniformsOffsets(const std::vector<unsigned>& indices) const
{
	auto result = getActiveUniformParameters(indices, GL_UNIFORM_OFFSET);
	return std::vector<unsigned>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformSize(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_SIZE);
}
//-----------------------------------------------------------------------------
std::vector<int> ShaderProgram::GetActiveUniformsSizes(const std::vector<unsigned>& indices) const
{
	return getActiveUniformParameters(indices, GL_UNIFORM_SIZE);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformBlockIndex(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_BLOCK_INDEX);
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveUniformsBlockIndices(const std::vector<unsigned>& indices) const
{
	auto result = getActiveUniformParameters(indices, GL_UNIFORM_BLOCK_INDEX);
	return std::vector<unsigned>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformArrayStride(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_ARRAY_STRIDE);
}
//-----------------------------------------------------------------------------
std::vector<int> ShaderProgram::GetActiveUniformsArrayStrides(const std::vector<unsigned>& indices) const
{
	return getActiveUniformParameters(indices, GL_UNIFORM_ARRAY_STRIDE);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformMatrixStride(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_MATRIX_STRIDE);
}
//-----------------------------------------------------------------------------
std::vector<int> ShaderProgram::GetActiveUniformsMatrixStrides(const std::vector<unsigned>& indices) const
{
	return getActiveUniformParameters(indices, GL_UNIFORM_MATRIX_STRIDE);
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformIsRowMajor(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_IS_ROW_MAJOR) != 0;
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformAtomicCounterBufferIndex(const unsigned index) const
{
	return getActiveUniformParameter(index, GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX);
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveUniformsAtomicCounterBufferIndices(const std::vector<unsigned>& indices) const
{
	auto result = getActiveUniformParameters(indices, GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX);
	return std::vector<GLuint>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
std::vector<int> ShaderProgram::GetActiveUniformsNameLengths(const std::vector<unsigned>& indices) const
{
	return getActiveUniformParameters(indices, GL_UNIFORM_NAME_LENGTH);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetUniformBlockIndex(const char* name) const
{
	// TODO: проверить что выдает эта функция и GetUniformBlockIndex(unsigned) - если разное, то переименовать
	return glGetUniformBlockIndex(m_id, name);
}
//-----------------------------------------------------------------------------
std::string ShaderProgram::GetActiveUniformBlockName(const unsigned index) const
{
	std::string result;
	result.resize(GetActiveUniformBlockNameLength(index));
	GL_CHECK(glGetActiveUniformBlockName(m_id, index, static_cast<GLsizei>(result.size()), nullptr, &result[0]));
	return result;
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformBlockBinding(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_BINDING);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformBlockDataSize(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_DATA_SIZE);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetActiveUniformBlockNameLength(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_NAME_LENGTH);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformBlockUniformCount(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS);
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByVertexShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByFragmentShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByComputeShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByGeometryShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByTessellationControlShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetActiveUniformBlockUniformIsReferencedByTessellationEvaluationShader(const unsigned index) const
{
	return getActiveUniformBlockParameter(index, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER) != 0;
}
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveUniformBlockUniformIndices(const unsigned index) const
{
	std::vector<GLint> result(GetActiveUniformBlockUniformCount(index));
	glGetActiveUniformBlockiv(m_id, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &result[0]);
	return std::vector<unsigned>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
unsigned ShaderProgram::GetActiveAtomicCounterBufferDataSize(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
int ShaderProgram::GetActiveAtomicCounterBufferCounters(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS);
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByVertexShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByFragmentShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByComputeShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByGeometryShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByTessellationControlShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 42
bool ShaderProgram::GetActiveAtomicCounterBufferIsReferencedByTessellationEvaluationShader(const unsigned index) const
{
	return getActiveAtomicCounterBufferParameter(index, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER) != 0;
}
#endif
//-----------------------------------------------------------------------------
std::vector<unsigned> ShaderProgram::GetActiveAtomicCounterBufferCounterIndices(const unsigned index) const
{
	std::vector<GLint> result(GetActiveUniformBlockUniformCount(index));
	glGetActiveUniformBlockiv(m_id, index, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES, &result[0]);
	return std::vector<GLuint>(result.begin(), result.end());
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, int value)
{
	glUniform1i(var.id, value);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, float value)
{
	glUniform1f(var.id, value);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, const glm::vec2& v)
{
	glUniform2fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, float x, float y, float z)
{
	glUniform3f(var.id, x, y, z);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, const glm::vec3& v)
{
	glUniform3fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, const glm::vec4& v)
{
	glUniform4fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, const glm::mat3& mat)
{
	glUniformMatrix3fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformLocation var, const glm::mat4& mat)
{
	glUniformMatrix4fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniformBlockBinding(const unsigned index, const unsigned binding) const
{
	glUniformBlockBinding(m_id, index, binding);
}
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
void ShaderProgram::SetShaderStorageBlockBinding(const unsigned index, const unsigned binding) const
{
	glShaderStorageBlockBinding(m_id, index, binding);
}
#endif
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveAttributeCount() const
{
	return getParameter(GL_ACTIVE_ATTRIBUTES);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveAttributeMaxLength() const
{
	return getParameter(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformCount() const
{
	return getParameter(GL_ACTIVE_UNIFORMS);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformMaxLength() const
{
	return getParameter(GL_ACTIVE_UNIFORM_MAX_LENGTH);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformBlockCount() const
{
	return getParameter(GL_ACTIVE_UNIFORM_BLOCKS);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveUniformBlockMaxNameLength() const
{
	return getParameter(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetTransformFeedbackBufferMode() const
{
	return getParameter(GL_TRANSFORM_FEEDBACK_BUFFER_MODE);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetTransformFeedbackVaryingCount() const
{
	return getParameter(GL_TRANSFORM_FEEDBACK_VARYINGS);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetTransformFeedbackVaryingMaxLength() const
{
	return getParameter(GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetGeometryVerticesOut() const
{
	return getParameter(GL_GEOMETRY_VERTICES_OUT);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetGeometryInputType() const
{
	return getParameter(GL_GEOMETRY_INPUT_TYPE);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetGeometryOutputType() const
{
	return getParameter(GL_GEOMETRY_OUTPUT_TYPE);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetGeometryShaderInvocations() const
{
	return getParameter(GL_GEOMETRY_SHADER_INVOCATIONS);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetTessellationControlOutputVertexCount() const
{
	return getParameter(GL_TESS_CONTROL_OUTPUT_VERTICES);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetTessellationGenerationMode() const
{
	return getParameter(GL_TESS_GEN_MODE);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetTessellationGenerationSpacing() const
{
	return getParameter(GL_TESS_GEN_SPACING);
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetTessellationGenerationVertexOrder() const
{
	return getParameter(GL_TESS_GEN_VERTEX_ORDER);
}
//-----------------------------------------------------------------------------
bool ShaderProgram::GetTessellationGenerationPointMode() const
{
	return getParameter(GL_TESS_GEN_POINT_MODE) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::IsSeparable() const
{
	return getParameter(GL_PROGRAM_SEPARABLE) != 0;
}
//-----------------------------------------------------------------------------
bool ShaderProgram::IsBinaryRetrievable() const
{
	return getParameter(GL_PROGRAM_BINARY_RETRIEVABLE_HINT) != 0;
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetActiveAtomicCounterBufferCount() const
{
	return getParameter(GL_ACTIVE_ATOMIC_COUNTER_BUFFERS);
}
//-----------------------------------------------------------------------------
int ShaderProgram::GetBinaryLength() const
{
	return getParameter(GL_PROGRAM_BINARY_LENGTH);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetAttributeLocation(const std::string& attributeName, const unsigned location) const
{
	// Note: Link must be called after. Always prefer to specify this explicitly in glsl whenever possible.
	glBindAttribLocation(m_id, location, attributeName.c_str());
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::GetAttributeLocation(const std::string& attributeName) const
{
	return glGetAttribLocation(m_id, attributeName.c_str());
}
//-----------------------------------------------------------------------------
std::tuple<std::string, unsigned, int> ShaderProgram::GetActiveAttribute(const unsigned location) const
{
	std::tuple<std::string, unsigned, int> result;
	auto& name = std::get<0>(result);
	auto& type = std::get<1>(result);
	auto& size = std::get<2>(result);
	name.resize(GetActiveAttributeMaxLength());
	glGetActiveAttrib(m_id, location, static_cast<GLsizei>(name.size()), nullptr, &size, &type, &name[0]);
	return result;
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetTransformFeedbackVaryings(const std::vector<std::string>& varyings, const unsigned bufferMode)
{
	std::vector<const char*> varyings_c(varyings.size());
	std::transform(varyings.begin(), varyings.end(), varyings_c.begin(), [&](const std::string& varying)
		{
			return varying.c_str();
		});
	glTransformFeedbackVaryings(m_id, static_cast<GLsizei>(varyings.size()), varyings_c.data(), bufferMode);
}
//-----------------------------------------------------------------------------
std::tuple<std::string, unsigned, int> ShaderProgram::GetTransformFeedbackVarying(const unsigned index) const
{
	std::tuple<std::string, unsigned, int> result;
	auto& name = std::get<0>(result);
	auto& type = std::get<1>(result);
	auto& size = std::get<2>(result);
	name.resize(GetTransformFeedbackVaryingMaxLength());
	glGetTransformFeedbackVarying(m_id, index, static_cast<GLsizei>(name.size()), nullptr, &size, &type, &name[0]);
	return result;
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetFragDataLocation(const unsigned colorNumber, const std::string& name) const
{
	glBindFragDataLocation(m_id, colorNumber, name.c_str());
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetFragDatLocationIndexed(const unsigned colorNumber, const unsigned index, const std::string& name) const
{
	glBindFragDataLocationIndexed(m_id, colorNumber, index, name.c_str());
}
//-----------------------------------------------------------------------------
GLuint ShaderProgram::GetFragDataLocation(const std::string& name) const
{
	return glGetFragDataLocation(m_id, name.c_str());
}
//-----------------------------------------------------------------------------
GLuint ShaderProgram::GetFragDataIndex(const std::string& name) const
{
	return glGetFragDataIndex(m_id, name.c_str());
}
//-----------------------------------------------------------------------------
bool ShaderProgram::IsSlowValid() const
{
	if (!IsValid()) return false;
	glValidateProgram(m_id);
	return getParameter(GL_VALIDATE_STATUS) != 0;
}
//-----------------------------------------------------------------------------
unsigned ShaderProgram::createShader(ShaderType type, const std::string& shaderString) const
{
	if (shaderString.empty() || shaderString == "") return 0;
			
	const GLenum glShaderType = translate(type);
	const GLuint shaderId = glCreateShader(glShaderType);

	{
		const char* shaderText = shaderString.data();
		const int32_t lenShaderText = shaderString.size();
		GL_CHECK(glShaderSource(shaderId, 1, &shaderText, &lenShaderText));
	}
		
	GL_CHECK(glCompileShader(shaderId));

	GLint compiled = GL_FALSE;
	GL_CHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled));
	if (compiled == GL_FALSE)
	{
		GLint infoLogSize;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogSize);
		std::vector<GLchar> errorInfo(infoLogSize);
		glGetShaderInfoLog(shaderId, errorInfo.size(), nullptr, &errorInfo[0]);
		glDeleteShader(shaderId);

		std::string shaderName;
		switch (glShaderType)
		{
		case GL_VERTEX_SHADER: shaderName = "Vertex "; break;
		case GL_GEOMETRY_SHADER: shaderName = "Geometry "; break;
		case GL_FRAGMENT_SHADER: shaderName = "Fragment "; break;
		}
		LogError(shaderName + "Shader compilation failed : " + std::string(&errorInfo[0]) + ", Source: " + shaderString);
		return 0;
	}

	return shaderId;
}
//-----------------------------------------------------------------------------
int ShaderProgram::getParameter(const unsigned parameter) const
{
	GLint result;
	GL_CHECK(glGetProgramiv(m_id, parameter, &result));
	return result;
}
//-----------------------------------------------------------------------------
#if OPENGL_VERSION >= 43
int ShaderProgram::getInterfaceParameter(const unsigned interface, const unsigned parameter) const
{
	GLint result;
	GL_CHECK(glGetProgramInterfaceiv(m_id, interface, parameter, &result));
	return result;
}
#endif
//-----------------------------------------------------------------------------
int ShaderProgram::getActiveUniformParameter(const unsigned index, const unsigned parameter) const
{
	int result = 0;
	GL_CHECK(glGetActiveUniformsiv(m_id, 1, &index, parameter, &result));
	return result;
}
//-----------------------------------------------------------------------------
std::vector<int> ShaderProgram::getActiveUniformParameters(const std::vector<unsigned>& indices, const unsigned parameter) const
{
	std::vector<int> result(indices.size());
	GL_CHECK(glGetActiveUniformsiv(m_id, static_cast<GLsizei>(indices.size()), indices.data(), parameter, result.data()));
	return result;
}
//-----------------------------------------------------------------------------
int ShaderProgram::getActiveUniformBlockParameter(const unsigned index, const unsigned parameter) const
{
	int result = 0;
	GL_CHECK(glGetActiveUniformBlockiv(m_id, index, parameter, &result));
	return result;
}
//-----------------------------------------------------------------------------
int ShaderProgram::getActiveAtomicCounterBufferParameter(const unsigned index, const unsigned parameter) const
{
	GLint result;
	GL_CHECK(glGetActiveAtomicCounterBufferiv(m_id, index, parameter, &result));
	return result;
}
//-----------------------------------------------------------------------------
int ShaderProgram::getProgramStageParameter(const unsigned shaderType, const unsigned parameter) const
{
	GLint result;
	GL_CHECK(glGetProgramStageiv(m_id, shaderType, parameter, &result));
	return result;
}
//-----------------------------------------------------------------------------
namespace ShaderLoader
{
	std::unordered_map<std::string, ShaderProgram> FileShaderPrograms;

	bool ReplaceInclude(std::string& line, const std::string& assetFile)
	{
		std::string basePath = "";
		size_t lastFS = assetFile.rfind("/");
		size_t lastBS = assetFile.rfind("\\");
		size_t lastS = (size_t)std::max((int)lastFS, (int)lastBS);
		if (!(lastS == std::string::npos))basePath = assetFile.substr(0, lastS) + "/";

		size_t firstQ = line.find("\"");
		size_t lastQ = line.rfind("\"");
		if ((firstQ == std::string::npos) ||
			(lastQ == std::string::npos) ||
			lastQ <= firstQ)
		{
			LogError("invalid include syntax.\n" + line);
			return false;
		}
		firstQ++;
		std::string path = basePath + line.substr(firstQ, lastQ - firstQ);

		std::ifstream shaderFile;
		shaderFile.open(path);
		if (!shaderFile)
		{
			LogError("Opening shader file \"" + path + "\" failed.");
			return false;
		}
		std::string ret;
		std::string extractedLine;
		while (shaderFile.eof() == false)
		{
			//Get the line
			getline(shaderFile, extractedLine);

			//Includes
			if (extractedLine.find("#include") != std::string::npos)
			{
				if (!(ReplaceInclude(extractedLine, assetFile)))
				{
					LogError("Opening shader file failed.");
					return false;
				}
			}

			ret += extractedLine;
			ret += "\n";
		}
		shaderFile.close();
		line = ret;
		return true;
	}

	void Destroy()
	{
		for (auto it = FileShaderPrograms.begin(); it != FileShaderPrograms.end(); ++it)
			it->second.Destroy();
		FileShaderPrograms.clear();
	}
	
	ShaderProgram* Load(const char* name)
	{
		auto it = FileShaderPrograms.find(name);
		if (it != FileShaderPrograms.end())
		{
			return &it->second;
		}
		else
		{
			LogPrint("Load shader programs: " + std::string(name));

			std::string vertSource;
			std::string geoSource;
			std::string fragSource;

			enum ParseState {
				INIT,
				VERT,
				GEO,
				FRAG
			} state = ParseState::INIT;
			bool useGeo = false;
			std::string extractedLine;
			std::ifstream shaderFile;
			shaderFile.open(name);
			if (!shaderFile)
			{
				LogError("Opening shader file failed.");
				return nullptr;
			}
			while (shaderFile.eof() == false)
			{
				//Get the line
				getline(shaderFile, extractedLine);

				//Includes
				if (extractedLine.find("#include") != std::string::npos)
				{
					if (!(ReplaceInclude(extractedLine, name)))
					{
						LogError("Opening shader file failed.");
						return nullptr;
					}
				}

				//Precompile types
				switch (state)
				{
				case INIT:
					if (extractedLine.find("<VERTEX>") != std::string::npos)
					{
						state = ParseState::VERT;
					}
					if (extractedLine.find("<GEOMETRY>") != std::string::npos)
					{
						useGeo = true;
						state = ParseState::GEO;
					}
					if (extractedLine.find("<FRAGMENT>") != std::string::npos)
					{
						state = ParseState::FRAG;
					}
					break;
				case VERT:
					if (extractedLine.find("</VERTEX>") != std::string::npos)
					{
						state = ParseState::INIT;
						break;
					}
					vertSource += extractedLine;
					vertSource += "\n";
					break;
				case GEO:
					if (extractedLine.find("</GEOMETRY>") != std::string::npos)
					{
						state = ParseState::INIT;
						break;
					}
					geoSource += extractedLine;
					geoSource += "\n";
					break;
				case FRAG:
					if (extractedLine.find("</FRAGMENT>") != std::string::npos)
					{
						state = ParseState::INIT;
						break;
					}
					fragSource += extractedLine;
					fragSource += "\n";
					break;
				}
			}
			shaderFile.close();

			ShaderProgram shaders;
			if (!shaders.CreateFromMemories(vertSource, geoSource, fragSource) || !shaders.IsValid())
				return nullptr;

			FileShaderPrograms[name] = shaders;
			return &FileShaderPrograms[name];
		}
	}

	bool IsLoad(const ShaderProgram& shaderProgram)
	{
		for (auto it = FileShaderPrograms.begin(); it != FileShaderPrograms.end(); ++it)
		{
			if (shaderProgram == it->second)
				return true;
		}
		return false;
	}
} // ShaderManager
//-----------------------------------------------------------------------------
//=============================================================================
// Image
//=============================================================================
//-----------------------------------------------------------------------------
Image::Image(Image&& imageRef) noexcept
{
	moveData(std::move(imageRef));
}
//-----------------------------------------------------------------------------
Image::~Image()
{
	Destroy();
}
//-----------------------------------------------------------------------------
Image& Image::operator=(Image&& imageRef) noexcept
{
	moveData(std::move(imageRef));
	return *this;
}
//-----------------------------------------------------------------------------
bool Image::Create(uint16_t width, uint16_t height, uint8_t channels, const std::vector<uint8_t>& pixelData)
{
	if (width == 0 || height == 0 || channels == 0 || channels > 4)
		return false;
	// TODO: возвращать ошибку если width/height слишком большое

	Destroy();

	m_width = width;
	m_height = height;
	m_comps = channels;

	if (pixelData.empty()) // залить белым
	{
		const size_t imageDataSize = width * height * channels;
		m_pixels = std::move(std::vector<uint8_t>(imageDataSize, 255));
	}
	else  // скопировать данные
		m_pixels = pixelData;

	return false;
}
//-----------------------------------------------------------------------------
bool Image::Load(const char* fileName, ImagePixelFormat desiredFormat, bool verticallyFlip)
{
	Destroy();

	int desiredСhannels = STBI_default;
	if (desiredFormat == ImagePixelFormat::R_U8) desiredСhannels = STBI_grey;
	if (desiredFormat == ImagePixelFormat::RG_U8) desiredСhannels = STBI_grey_alpha;
	if (desiredFormat == ImagePixelFormat::RGB_U8) desiredСhannels = STBI_rgb;
	if (desiredFormat == ImagePixelFormat::RGBA_U8) desiredСhannels = STBI_rgb_alpha;

	int width = 0;
	int height = 0;
	int comps = 0;

	stbi_set_flip_vertically_on_load(verticallyFlip ? 1 : 0);
#if 0
	int len = 0;
	std::vector<char> data = FileSystem::Fileload(fileName, &len);
	if (data.empty() || len <= 0)
		return false;
		
	 stbi_uc* pixelData = stbi_load_from_memory((const stbi_uc*)data.data(), len, &width, &height, &comps, desiredСhannels);
#else
	stbi_uc* pixelData = stbi_load(fileName, &width, &height, &comps, desiredСhannels);
#endif

	// TODO: проверку что width влезет в m_width (и для остальных). 
	m_width = width;
	m_height = height;
	m_comps = comps;

	if (!pixelData || m_comps < STBI_grey || m_comps > STBI_rgb_alpha || m_width == 0 || m_height == 0)
	{
		LogError("Image loading failed! Filename='" + std::string(fileName) + "'");
		stbi_image_free((void*)pixelData);
		return false;
	}

	m_comps = desiredСhannels ? desiredСhannels : m_comps;

	const size_t imageDataSize = m_width * m_height * m_comps;
	m_pixels.assign(pixelData, pixelData + imageDataSize);

	stbi_image_free(pixelData);

	return true;
}
//-----------------------------------------------------------------------------
void Image::Destroy()
{
	m_pixels.clear();
	m_width = m_height = m_comps = 0;
}
//-----------------------------------------------------------------------------
bool Image::IsTransparent() const
{
	bool isTransparent = false;
	// TODO: может быть медленно, проверить скорость и поискать другое решение
	if (m_comps == 4) // TODO: сделать еще и для 2
	{
		for (int i = 0; i < GetSizeData(); i += 4)
		{
			//uint8_t r = tempImage[i];
			//uint8_t g = tempImage[i + 1];
			//uint8_t b = tempImage[i + 2];
			const uint8_t& a = m_pixels[i + 3];
			if (a < 255)
			{
				isTransparent = true;
				break;
			}
		}
	}

	return isTransparent;
}
//-----------------------------------------------------------------------------
glm::vec3 Image::Bilinear(const glm::vec2& uv)
{
	float w = m_width, h = m_height, u = uv.x, v = uv.y;

	float u1 = (int)u, v1 = (int)v, u2 = std::min(u1 + 1, w - 1), v2 = std::min(v1 + 1, h - 1);
	float c1 = u - u1, c2 = v - v1;
	uint8_t* p1 = &m_pixels[m_comps * (int)(u1 + v1 * m_width)];
	uint8_t* p2 = &m_pixels[m_comps * (int)(u2 + v1 * m_width)];
	uint8_t* p3 = &m_pixels[m_comps * (int)(u1 + v2 * m_width)];
	uint8_t* p4 = &m_pixels[m_comps * (int)(u2 + v2 * m_width)];
	glm::vec3 A = {p1[0], p1[1], p1[2]};
	glm::vec3 B = {p2[0], p2[1], p2[2]};
	glm::vec3 C = {p3[0], p3[1], p3[2]};
	glm::vec3 D = {p4[0], p4[1], p4[2]};
	return Mix(Mix(A, B, c1), Mix(C, D, c1), c2);
}
//-----------------------------------------------------------------------------
void Image::moveData(Image&& imageRef)
{
	m_width = imageRef.m_width;
	m_height = imageRef.m_height;
	m_comps = imageRef.m_comps;
	m_pixels = std::move(imageRef.m_pixels);
}
//-----------------------------------------------------------------------------
//=============================================================================
// Texture
//=============================================================================
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
bool Texture2D::Create(const char* fileName, bool verticallyFlip, const Texture2DInfo& textureInfo)
{
	Image tempImage;
	if (!tempImage.Load(fileName, ImagePixelFormat::FromSource, verticallyFlip))
	{
		LogError("Texture loading failed! Filename='" + std::string(fileName) + "'");
		return false;
	}

	return Create(&tempImage, textureInfo);
}
//-----------------------------------------------------------------------------
bool Texture2D::Create(Image* image, const Texture2DInfo& textureInfo)
{
	if (!image || !image->IsValid()) 
		return false;

	const int nrChannels = image->GetChannels();

	Destroy();

	Texture2DCreateInfo createInfo;
	{
		// проверить на прозрачность
		createInfo.isTransparent = image->IsTransparent();

		createInfo.format = TexelsFormat::RGB_U8;
		if (nrChannels == STBI_grey) createInfo.format = TexelsFormat::R_U8;
		else if (nrChannels == STBI_grey_alpha) createInfo.format = TexelsFormat::RG_U8;
		else if (nrChannels == STBI_rgb) createInfo.format = TexelsFormat::RGB_U8;
		else if (nrChannels == STBI_rgb_alpha) createInfo.format = TexelsFormat::RGBA_U8;

		createInfo.width = static_cast<uint16_t>(image->GetWidth());
		createInfo.height = static_cast<uint16_t>(image->GetHeight());
		createInfo.depth = 1;
		createInfo.pixelData = image->GetData();
	}

	return Create(createInfo);
}
//-----------------------------------------------------------------------------
bool Texture2D::Create(const Texture2DCreateInfo& createInfo, const Texture2DInfo& textureInfo)
{
	Destroy();

	isTransparent = createInfo.isTransparent;
	m_width = createInfo.width;
	m_height = createInfo.height;

	// save prev pixel store state
	GLint Alignment = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &Alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// gen texture res
	glGenTextures(1, &m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_id);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, translate(textureInfo.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, translate(textureInfo.wrapT));

	// set texture filtering parameters
	TextureMinFilter minFilter = textureInfo.minFilter;
	if (!textureInfo.mipmap)
	{
		if (textureInfo.minFilter == TextureMinFilter::NearestMipmapNearest) minFilter = TextureMinFilter::Nearest;
		else if (textureInfo.minFilter != TextureMinFilter::Nearest) minFilter = TextureMinFilter::Linear;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translate(minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translate(textureInfo.magFilter));

	// set texture format
	GLenum format = GL_RGB;
	GLint internalFormat = GL_RGB;
	GLenum oglType = GL_UNSIGNED_BYTE;
	getTextureFormatType(createInfo.format, GL_TEXTURE_2D, format, internalFormat, oglType);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, oglType, createInfo.pixelData);
	if (textureInfo.mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	// restore prev state
#if USE_OPENGL_CACHE_STATE
	glBindTexture(GL_TEXTURE_2D, RendererState::currentTexture2D[0]);
#endif
	glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);

	return true;
}
//-----------------------------------------------------------------------------
void Texture2D::Destroy()
{
	if (m_id > 0)
	{
#if USE_OPENGL_CACHE_STATE
		for (unsigned i = 0; i < MAXTEXTURE; i++)
		{
			if (RendererState::currentTexture2D[i] == m_id)
				Texture2D::UnBind(i);
		}
#endif
		if (!TextureLoader::IsLoad(*this)) // TODO: не удалять текстуру если она загружена через менеджер, в будущем сделать подсчет ссылок и удалять если нет
			glDeleteTextures(1, &m_id);
		m_id = 0;
	}
}
//-----------------------------------------------------------------------------
void Texture2D::Bind(unsigned slot) const
{
#if USE_OPENGL_CACHE_STATE
	if (RendererState::currentTexture2D[slot] == m_id)
		return;

	RendererState::currentTexture2D[slot] = m_id;
#endif
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_id);
}
//-----------------------------------------------------------------------------
void Texture2D::UnBind(unsigned slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
#if USE_OPENGL_CACHE_STATE
	RendererState::currentTexture2D[slot] = 0;
#endif
}
//-----------------------------------------------------------------------------
namespace TextureLoader
{
	std::unordered_map<std::string, Texture2D> FileTextures;

	void Destroy()
	{
		for (auto it = FileTextures.begin(); it != FileTextures.end(); ++it)
			it->second.Destroy();
		FileTextures.clear();
	}

	Texture2D* LoadTexture2D(const char* fileName, bool verticallyFlip, const Texture2DInfo& textureInfo)
	{
		auto it = FileTextures.find(fileName);
		if (it != FileTextures.end())
		{
			return &it->second;
		}
		else
		{
			LogPrint("Load texture: " + std::string(fileName));

			Texture2D texture;
			if (!texture.Create(fileName, verticallyFlip, textureInfo) || !texture.IsValid())
				return nullptr;

			FileTextures[fileName] = texture;
			return &FileTextures[fileName];
		}
	}

	bool IsLoad(const Texture2D& texture)
	{
		for (auto it = FileTextures.begin(); it != FileTextures.end(); ++it)
		{
			if (texture == it->second)
				return true;
		}
		return false;
	}
}
//-----------------------------------------------------------------------------
bool VertexBuffer::Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data)
{
	if (m_id > 0) Destroy();

	m_vertexCount = vertexCount;
	m_vertexSize = vertexSize;
	m_usage = usage;

#if USE_OPENGL_DSA
	GL_CHECK(glCreateBuffers(1, &m_id));
	GL_CHECK(glNamedBufferData(m_id, static_cast<GLsizeiptr>(vertexCount * m_vertexSize), data, translate(m_usage)));
#else
	// Backup the currently bound OpenGL array buffer
	GLint openGLArrayBufferBackup = 0;
	GL_CHECK(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &openGLArrayBufferBackup));

	GL_CHECK(glGenBuffers(1, &m_id));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_id));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptrARB>(vertexCount * vertexSize), data, translate(m_usage)));

	// Be polite and restore the previous bound OpenGL array buffer
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(openGLArrayBufferBackup)));
#endif
	return true;
}
//-----------------------------------------------------------------------------
void VertexBuffer::Destroy()
{
	GLint openGLArrayBufferBackup = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &openGLArrayBufferBackup);
	if (static_cast<unsigned>(openGLArrayBufferBackup) == m_id) glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (m_id) glDeleteBuffers(1, &m_id);
	m_id = 0;
}
//-----------------------------------------------------------------------------
void VertexBuffer::Update(unsigned offset, unsigned vertexCount, unsigned vertexSize, const void* data)
{
	Bind();

	if (m_vertexCount != vertexCount || m_vertexSize != vertexSize || m_usage != RenderResourceUsage::Dynamic)
	{
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptrARB>(vertexCount * vertexSize), data, translate(RenderResourceUsage::Dynamic)));
		m_usage = RenderResourceUsage::Dynamic;
	}
	else
	{
		GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, static_cast<GLsizeiptrARB>(vertexCount * vertexSize), data));
	}
	m_vertexCount = vertexCount;
	m_vertexSize = vertexSize;
	
	VertexArrayBuffer::UnBind();
}
//-----------------------------------------------------------------------------
void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void IndexBuffer::Destroy()
{
	glDeleteBuffers(1, &m_id);
	m_id = 0;
}
//-----------------------------------------------------------------------------
void IndexBuffer::Update(unsigned offset, unsigned indexCount, unsigned indexSize, const void* data)
{
	Bind();

	if (m_indexCount != indexCount || m_indexSize != indexSize || m_usage != RenderResourceUsage::Dynamic)
	{
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptrARB>(indexCount * indexSize), data, translate(RenderResourceUsage::Dynamic)));
		m_usage = RenderResourceUsage::Dynamic;
	}
	else
	{
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, static_cast<GLsizeiptrARB>(indexCount * indexSize), data));
	}
	m_indexCount = indexCount;
	m_indexSize = indexSize;

	VertexArrayBuffer::UnBind();
}
//-----------------------------------------------------------------------------
void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
//-----------------------------------------------------------------------------
//=============================================================================
// VertexArrayBuffer
//=============================================================================
//-----------------------------------------------------------------------------
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

	glBindVertexArray(RendererState::currentVAO);
	return true;
}
//-----------------------------------------------------------------------------
bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs, const std::vector<VertexAttributeRaw>& instanceAttribs)
{
	if (!Create(vbo, ibo, attribs))
		return false;

	SetInstancedBuffer(instanceBuffer, instanceAttribs);
	return true;
}
//-----------------------------------------------------------------------------
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

	glBindVertexArray(RendererState::currentVAO);
	return true;
}
//-----------------------------------------------------------------------------
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
			// TODO: сделать возможность указания локации атрибутов. сейчас подразумевается что первый атрибут - 0, второй - 1 и о порядку
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
//-----------------------------------------------------------------------------
void VertexArrayBuffer::Destroy()
{
	if (m_id > 0 && RendererState::currentVAO == m_id)
	{
		if (m_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		RendererState::currentVAO = 0;
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_id);
	}
	m_id = 0;
	m_instanceBuffer = nullptr;
}
//-----------------------------------------------------------------------------
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
				// TODO: проверить
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
			}
		}
	}
#endif

	glBindVertexArray(RendererState::currentVAO);
}
//-----------------------------------------------------------------------------
void VertexArrayBuffer::Draw(PrimitiveDraw primitive, uint32_t instanceCount)
{
	if (instanceCount == 0) return;

	if (RendererState::currentVAO != m_id)
	{
		RendererState::currentVAO = m_id;
		glBindVertexArray(m_id);
		m_vbo->Bind();
		if (m_ibo) m_ibo->Bind();
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
//-----------------------------------------------------------------------------
void VertexArrayBuffer::DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex)
{
	if (!m_ibo) return;

	if (RendererState::currentVAO != m_id)
	{
		RendererState::currentVAO = m_id;
		glBindVertexArray(m_id);
		m_vbo->Bind();
		if (m_ibo) m_ibo->Bind();
	}

	const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElementsBaseVertex(translate(primitive), indexCount, indexSizeType, (void*)(m_ibo->GetIndexSize() * baseIndex), baseVertex);
}
//-----------------------------------------------------------------------------
void VertexArrayBuffer::UnBind()
{
	RendererState::currentVAO = 0;
	glBindVertexArray(0);
}
//-----------------------------------------------------------------------------
//=============================================================================
// FrameBuffer
//=============================================================================
//-----------------------------------------------------------------------------
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

	if (!checkFramebuffer())
	{
		LogError("Framebuffer is not complete!");
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RendererState::currentFrameBuffer = nullptr;
	const float aspect = (float)width / (float)height;
	const float FOVY = glm::atan(glm::tan(glm::radians(RendererState::perspectiveFOV) / 2.0f) / aspect) * 2.0f;
	m_projectionMatrix = glm::perspective(FOVY, aspect, RendererState::perspectiveNear, RendererState::perspectiveFar);

	return true;
}
//-----------------------------------------------------------------------------
void FrameBuffer::Destroy()
{
	if (RendererState::currentFrameBuffer == this) MainFrameBufferBind();

	glDeleteTextures(1, &m_texColorBuffer);
	glDeleteRenderbuffers(1, &m_rbo);
	glDeleteFramebuffers(1, &m_id);
}
//-----------------------------------------------------------------------------
void FrameBuffer::Bind(const glm::vec3& color)
{
	if (RendererState::currentFrameBuffer != this)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glViewport(0, 0, m_width, m_height);
		RendererState::currentFrameBuffer = this;
	}
	glClearColor(color.x, color.y, color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
//-----------------------------------------------------------------------------
void FrameBuffer::MainFrameBufferBind()
{
	if (RendererState::currentFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
	glClearColor(RendererState::ClearColor.x, RendererState::ClearColor.y, RendererState::ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RendererState::currentFrameBuffer = nullptr;
}
//-----------------------------------------------------------------------------
void FrameBuffer::BindTextureBuffer()
{
	Texture2D::UnBind(); // TODO:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
}
//-----------------------------------------------------------------------------
bool FrameBuffer::checkFramebuffer()
{
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		return true;

	std::string strStatus = "";
	switch (status)
	{
	case GL_FRAMEBUFFER_UNDEFINED: strStatus = "GL_FRAMEBUFFER_UNDEFINED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
	case GL_FRAMEBUFFER_UNSUPPORTED: strStatus = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
	default: strStatus = "UNKNOWN"; break;
	}
	LogError("OpenGL Error = " + strStatus);

	return false;
}
//-----------------------------------------------------------------------------
const glm::mat4& GetCurrentProjectionMatrix()
{
	if (RendererState::currentFrameBuffer)
		return RendererState::currentFrameBuffer->GetProjectionMatrix();
	else
		return RendererState::projectionMatrix;
}
//-----------------------------------------------------------------------------
//=============================================================================
// Render System
//=============================================================================
//-----------------------------------------------------------------------------
bool RenderSystem::Create(const RenderSystem::CreateInfo& createInfo)
{
	const char* vendor = (const char*)glGetString(GL_VENDOR);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* version = (const char*)glGetString(GL_VERSION);
	const char* glslstr = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	LogPrint("OpenGL device information:");
	LogPrint("    > Renderer: " + std::string(renderer) + " (" + std::string(vendor) + ")");
	LogPrint("    > Version:  " + std::string(version));
	LogPrint("    > GLSL:     " + std::string(glslstr));

	bool mesa = false, intel = false, ati = false, nvidia = false;
	if (strstr(renderer, "Mesa") || strstr(version, "Mesa"))
	{
		mesa = true;
		if (strstr(renderer, "Intel")) intel = true;
	}
	else if (strstr(vendor, "NVIDIA"))
		nvidia = true;
	else if (strstr(vendor, "ATI") || strstr(vendor, "Advanced Micro Devices"))
		ati = true;
	else if (strstr(vendor, "Intel"))
		intel = true;

	//int glversion = 0;
	//uint32_t glmajorversion, glminorversion;
	//if (sscanf(version, " %u.%u", &glmajorversion, &glminorversion) != 2) glversion = 100;
	//else glversion = glmajorversion * 100 + glminorversion * 10;
	//if (glversion < 440)
	//{
	//	Fatal("OpenGL 4.4 or greater is required!");
	//	return false;
	//}

	LogPrint("OpenGL limits:");
	GLint capability = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &capability);
	LogPrint("    > GL_MAX_TEXTURE_SIZE: " + std::to_string(capability));
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &capability);
	LogPrint("    > GL_MAX_CUBE_MAP_TEXTURE_SIZE: " + std::to_string(capability));

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &capability);
	LogPrint("    > GL_MAX_VERTEX_UNIFORM_COMPONENTS: " + std::to_string(capability));
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &capability);
	LogPrint("    > GL_MAX_FRAGMENT_UNIFORM_COMPONENTS : " + std::to_string(capability));
	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &capability);
	LogPrint("    > GL_MAX_UNIFORM_LOCATIONS: " + std::to_string(capability));
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &capability);
	LogPrint("    > GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " + std::to_string(capability));


#if defined(_DEBUG)
	if ((glDebugMessageCallback != NULL) && (glDebugMessageControl != NULL))
	{
		glDebugMessageCallback(openglDebugMessageCallback, 0);
		// glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, 0, GL_TRUE); // TODO: Filter message

		// Debug context options:
		//  - GL_DEBUG_OUTPUT - Faster version but not useful for breakpoints
		//  - GL_DEBUG_OUTPUT_SYNCHRONUS - Callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	RendererState::ClearColor = createInfo.ClearColor;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glClearColor(RendererState::ClearColor.x, RendererState::ClearColor.y, RendererState::ClearColor.z, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, GetRenderWidth(), GetRenderHeight());

	RendererState::perspectiveFOV = createInfo.PerspectiveFOV;
	RendererState::perspectiveNear = createInfo.PerspectiveNear;
	RendererState::perspectiveFar = createInfo.PerspectiveFar;

	const float FOVY = glm::atan(glm::tan(glm::radians(RendererState::perspectiveFOV) / 2.0f) / GetRenderAspectRatio()) * 2.0f;
	RendererState::projectionMatrix = glm::perspective(FOVY, GetRenderAspectRatio(), RendererState::perspectiveNear, RendererState::perspectiveFar);

	return true;
}
//-----------------------------------------------------------------------------
void RenderSystem::Destroy()
{

}
//-----------------------------------------------------------------------------
void RenderSystem::SetFrameColor(const glm::vec3 clearColor)
{
	RendererState::ClearColor = clearColor;
	glClearColor(RendererState::ClearColor.x, RendererState::ClearColor.y, RendererState::ClearColor.z, 1.0f);
}
//-----------------------------------------------------------------------------
void RenderSystem::BeginFrame()
{
	if (RendererState::framebufferWidth != GetRenderWidth() || RendererState::framebufferHeight != GetRenderHeight())
	{
		RendererState::framebufferWidth = GetRenderWidth();
		RendererState::framebufferHeight = GetRenderHeight();
		glViewport(0, 0, RendererState::framebufferWidth, RendererState::framebufferHeight);
		glScissor(0, 0, RendererState::framebufferWidth, RendererState::framebufferHeight);
		const float FOVY = glm::atan(glm::tan(glm::radians(RendererState::perspectiveFOV) / 2.0f) / GetRenderAspectRatio()) * 2.0f;
		RendererState::projectionMatrix = glm::perspective(FOVY, GetRenderAspectRatio(), RendererState::perspectiveNear, RendererState::perspectiveFar);
	}

	//glClearDepthf(1.0f);
	//glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
//-----------------------------------------------------------------------------
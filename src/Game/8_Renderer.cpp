#include "stdafx.h"
#include "2_Base.h"
#include "3_Core.h"
#include "6_Platform.h"
#include "8_Renderer.h"

#include <stb/stb_image.h>
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
//=============================================================================
// OpenGL Core
//=============================================================================
//-----------------------------------------------------------------------------
void glCheckError(const char* func, const char* file, int line)
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
		LogError("OpenGL Error(" + error + ") in func: " + std::string(func) + " - " + std::string(file) + " (" + std::to_string(line) + ")");
	}
}
#define GL_CHECK(_func)                                       \
				{                                             \
					_func;                                    \
					glCheckError(#_func, __FILE__, __LINE__); \
				}
//-----------------------------------------------------------------------------
#if defined(_DEBUG)
void openglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept
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
glm::vec3 ClearColor; // TODO: в пространство имен
float perspectiveFOV = 0.0f;
float perspectiveNear = 0.01f;
float perspectiveFar = 1000.0f;
glm::mat4 projectionMatrix;
namespace
{
	int RenderWidth = 0;
	int RenderHeight = 0;
}
//=============================================================================
// Current Render State
//=============================================================================
#if USE_OPENGLCACHESTATE
namespace currentRenderState
{
	unsigned shaderProgram = 0;
	unsigned texture2D[MAXTEXTURE] = { 0 };
}
#endif
//-----------------------------------------------------------------------------
const char* glslTypeName(GLuint type)
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
		if (glShaderGeometry > 0)
			GL_CHECK(glAttachShader(m_id, glShaderGeometry));

		GL_CHECK(glLinkProgram(m_id));

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
	if (glShaderGeometry > 0)
		GL_CHECK(glDeleteShader(glShaderGeometry));

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
#if USE_OPENGLCACHESTATE
		if (currentRenderState::shaderProgram == m_id) UnBind();
#endif
		if (!ShaderLoader::IsLoad(*this)) // TODO: не удалять шейдер если он загружен через менеджер, в будущем сделать подсчет ссылок и удалять если нет
			glDeleteProgram(m_id);
		m_id = 0;
	}
}
//-----------------------------------------------------------------------------
void ShaderProgram::Bind()
{
#if USE_OPENGLCACHESTATE
	if (currentRenderState::shaderProgram == m_id)
		return;
	currentRenderState::shaderProgram = m_id;
#endif
	glUseProgram(m_id);
}
//-----------------------------------------------------------------------------
void ShaderProgram::UnBind()
{
#if USE_OPENGLCACHESTATE
	currentRenderState::shaderProgram = 0;
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

	uint32_t maxLength = base::Max(max0, max1);
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
			GL_CHECK(glGetProgramResourceiv(m_id, GL_PROGRAM_INPUT, i, base::Countof(typeProp), typeProp, 1, NULL, (GLint*)&type));
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

	uint32_t maxLength = base::Max(max0, max1);
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
			GL_CHECK(glGetProgramResourceiv(m_id, GL_UNIFORM, i, base::Countof(props), props, base::Countof(props), NULL, (GLint*)&vi
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

		num = base::Max(num, 1);

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
			if (m_numSamplers < base::Countof(m_sampler))
			{
				LogPrint("Sampler #" + std::to_string(m_numSamplers) + " at location " + std::to_string(loc));
				m_sampler[m_numSamplers] = loc;
				m_numSamplers++;
			}
			else
			{
				LogPrint("Too many samplers (max: " + std::to_string(base::Countof(m_sampler)) + ")! Sampler at location " + std::to_string(loc));
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
UniformLocation ShaderProgram::GetUniformVariable(const char* name)
{
	return { glGetUniformLocation(m_id, name) };
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
bool Image::Create(unsigned width, unsigned height, unsigned channels, const std::vector<uint8_t>& pixelData)
{
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
bool Image::Load(const char* fileName, bool verticallyFlip)
{
	Destroy();
#if 1
	int len = 0;
	std::vector<char> data = FileSystem::Fileload(fileName, &len);
	if (data.empty() || len <= 0)
		return false;

	stbi_set_flip_vertically_on_load(verticallyFlip ? 1 : 0);

	stbi_uc* pixelData = nullptr;
	int n = 0;
	//if (flags & IMAGE_R) n = 1;
	//if (flags & IMAGE_RG) n = 2;
	//if (flags & IMAGE_RGB) n = 3;
	//if (flags & IMAGE_RGBA) n = 4;
	//if (flags & IMAGE_FLOAT)
		//pixelData = (uint8_t*)stbi_loadf_from_memory((const stbi_uc*)data.data(), len, (int*)&m_w, (int*)&m_h, (int*)&m_comps, n);
	//else
		pixelData = stbi_load_from_memory((const stbi_uc*)data.data(), len, (int*)&m_width, (int*)&m_height, (int*)&m_comps, n);
	if (!pixelData)
	{
		LogError("Error loading image (" + std::string(fileName) + ")");
		stbi_image_free(pixelData);
		return false;
	}
	m_comps = n ? n : m_comps;

	const size_t imageDataSize = m_width * m_height * m_comps;

	m_pixels.assign(pixelData, pixelData + imageDataSize);

	stbi_image_free(pixelData);
#else
#endif
	return true;
}
//-----------------------------------------------------------------------------
void Image::Destroy()
{
	m_pixels.clear();
	m_width = m_height = m_comps = 0;
}
//-----------------------------------------------------------------------------
void Image::moveData(Image&& imageRef)
{
	m_width = imageRef.m_width;
	m_height = imageRef.m_height;
	m_comps = imageRef.m_comps;
	m_pixels = std::move(imageRef.m_pixels);

	// ref clear
	//imageRef.m_pixels = nullptr;
	imageRef.m_width = imageRef.m_height = imageRef.m_comps = 0;
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
	if (fileName == nullptr || fileName == "") return false;

	Destroy();

	Texture2DCreateInfo createInfo;
	{
		if (verticallyFlip)
			stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

		int width = 0;
		int height = 0;
		int nrChannels = 0;
		TexelsFormat format = TexelsFormat::RGB_U8;
		stbi_uc* pixelData = stbi_load(fileName, &width, &height, &nrChannels, 0);
		if (!pixelData || nrChannels < STBI_grey || nrChannels > STBI_rgb_alpha || width == 0 || height == 0)
		{
			LogError("Texture loading failed! Filename='" + std::string(fileName) + "'");
			stbi_image_free((void*)pixelData);
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
				//uint8_t r = pixelData[i];
				//uint8_t g = pixelData[i + 1];
				//uint8_t b = pixelData[i + 2];
				const uint8_t& a = pixelData[i + 3];
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
		createInfo.pixelData = pixelData;

		bool isValid = Create(createInfo);
		stbi_image_free((void*)pixelData);
		if (!isValid) return false;
	}

	return true;
}
//-----------------------------------------------------------------------------
bool Texture2D::Create(Image* image, const Texture2DInfo& textureInfo)
{
	if (!image || !image->IsValid()) return false;

	Destroy();

	Texture2DCreateInfo createInfo;
	{
		const int width = image->GetWidth();
		const int height = image->GetHeight();
		const int nrChannels = image->GetChannels();

		uint8_t* pixelData = image->GetData();
		if (!pixelData || nrChannels < STBI_grey || nrChannels > STBI_rgb_alpha || width == 0 || height == 0)
		{
			LogError("Texture loading failed!");
			return false;
		}

		TexelsFormat format = TexelsFormat::RGB_U8;
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
				//uint8_t r = pixelData[i];
				//uint8_t g = pixelData[i + 1];
				//uint8_t b = pixelData[i + 2];
				const uint8_t& a = pixelData[i + 3];
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
		createInfo.pixelData = pixelData;
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
#if USE_OPENGLCACHESTATE
	glBindTexture(GL_TEXTURE_2D, currentRenderState::texture2D[0]);
#endif
	glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);

	return true;
}
//-----------------------------------------------------------------------------
void Texture2D::Destroy()
{
	if (m_id > 0)
	{
#if USE_OPENGLCACHESTATE
		for (unsigned i = 0; i < MAXTEXTURE; i++)
		{
			if (currentRenderState::texture2D[i] == m_id)
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
#if USE_OPENGLCACHESTATE
	if (currentRenderState::texture2D[slot] == m_id)
		return;

	currentRenderState::texture2D[slot] = m_id;
#endif
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_id);
}
//-----------------------------------------------------------------------------
void Texture2D::UnBind(unsigned slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
#if USE_OPENGLCACHESTATE
	currentRenderState::texture2D[slot] = 0;
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
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptrARB>(vertexCount * m_vertexSize), data, translate(m_usage)));

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
void VertexBuffer::Update(unsigned offset, unsigned size, const void* data)
{
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
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
void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
//-----------------------------------------------------------------------------
//=============================================================================
// Render System
//=============================================================================
//-----------------------------------------------------------------------------
bool RenderSystem::Create(const RenderSystem::CreateInfo& createInfo)
{
	LogPrint("OpenGL device information:");
	LogPrint("    > Vendor:   " + std::string((const char*)glGetString(GL_VENDOR)));
	LogPrint("    > Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
	LogPrint("    > Version:  " + std::string((const char*)glGetString(GL_VERSION)));
	LogPrint("    > GLSL:     " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));

	LogPrint("OpenGL limits:");
	GLint capability = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &capability);
	LogPrint("    > GL_MAX_VERTEX_UNIFORM_COMPONENTS: " + std::to_string(capability));
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &capability);
	LogPrint("    > GL_MAX_FRAGMENT_UNIFORM_COMPONENTS : " + std::to_string(capability));
	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &capability);
	LogPrint("    > GL_MAX_UNIFORM_LOCATIONS: " + std::to_string(capability));

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
//-----------------------------------------------------------------------------
void RenderSystem::Destroy()
{

}
//-----------------------------------------------------------------------------
void RenderSystem::BeginFrame()
{
	if (RenderWidth != GetFrameBufferWidth() || RenderHeight != GetFrameBufferHeight())
	{
		RenderWidth = GetFrameBufferWidth();
		RenderHeight = GetFrameBufferHeight();
		glViewport(0, 0, RenderWidth, RenderHeight);
		glScissor(0, 0, RenderWidth, RenderHeight);
		const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / GetFrameBufferAspectRatio()) * 2.0f;
		projectionMatrix = glm::perspective(FOVY, GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
//-----------------------------------------------------------------------------
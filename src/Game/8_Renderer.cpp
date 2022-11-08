#include "stdafx.h"
#include "2_Base.h"
#include "3_Core.h"
#include "6_Platform.h"
#include "8_Renderer.h"
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
void glCheckError(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		LogError(error + " | " + std::string(file) + " (" + std::to_string(line) + ")");
	}
}
#define GL_CHECK(_func)                               \
				{                                     \
					_func;                            \
					glCheckError(__FILE__, __LINE__); \
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


glm::vec3 ClearColor; // TODO: � ������������ ����
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
// OpenGL Core
//=============================================================================
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
// Current Render State
//=============================================================================
namespace currentRenderState
{
	unsigned shaderProgram = 0;
}
//=============================================================================
// ShaderProgram
//=============================================================================
//-----------------------------------------------------------------------------
bool ShaderProgram::CreateFromMemories(const std::string& vertexShaderMemory, const std::string& fragmentShaderMemory)
{
	if (vertexShaderMemory.empty() || fragmentShaderMemory.empty()) return false;
	if (vertexShaderMemory == "" || fragmentShaderMemory == "") return false;
	if (m_id > 0) Destroy();

	const GLuint glShaderVertex = createShader(ShaderType::Vertex, vertexShaderMemory);
	const GLuint glShaderFragment = createShader(ShaderType::Fragment, fragmentShaderMemory);

	if (glShaderVertex > 0 && glShaderFragment > 0)
	{
		m_id = glCreateProgram();
		GL_CHECK(glAttachShader(m_id, glShaderVertex));
		GL_CHECK(glAttachShader(m_id, glShaderFragment));

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
		if (currentRenderState::shaderProgram == m_id) UnBind();
		glDeleteProgram(m_id);
		m_id = 0;
	}
}
//-----------------------------------------------------------------------------
void ShaderProgram::Bind()
{
	if (currentRenderState::shaderProgram != m_id)
	{
		currentRenderState::shaderProgram = m_id;
		glUseProgram(currentRenderState::shaderProgram);
	}
}
//-----------------------------------------------------------------------------
void ShaderProgram::UnBind()
{
	currentRenderState::shaderProgram = 0;
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

	// TODO: ������� ���������� ������� �� location
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
		
	GLenum glShaderType = 0;
	if (type == ShaderType::Vertex) glShaderType = GL_VERTEX_SHADER;
	else if (type == ShaderType::Fragment) glShaderType = GL_FRAGMENT_SHADER;
	
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
		LogError("Shader compilation failed : " + std::string(&errorInfo[0]) + ", Source: " + shaderString);
		return 0;
	}

	return shaderId;
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
		const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / GetFrameBufferAspectRatio()) * 2.0f;
		projectionMatrix = glm::perspective(FOVY, GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
//-----------------------------------------------------------------------------
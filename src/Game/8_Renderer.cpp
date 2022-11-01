#include "stdafx.h"
#include "2_Base.h"
#include "3_Core.h"
#include "8_Renderer.h"

//=============================================================================
// OpenGL Core
//=============================================================================
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

	const GLuint shaderVertex = createShader(ShaderType::Vertex, vertexShaderMemory);
	const GLuint shaderFragment = createShader(ShaderType::Fragment, fragmentShaderMemory);

	if (shaderVertex > 0 && shaderFragment > 0)
	{
		m_id = glCreateProgram();
		GL_CHECK(glAttachShader(m_id, shaderVertex));
		GL_CHECK(glAttachShader(m_id, shaderFragment));

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

	GL_CHECK(glDeleteShader(shaderVertex));
	GL_CHECK(glDeleteShader(shaderFragment));

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

	// TODO: сделать сортировку вектора по location
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

	GLint success = 0;
	GL_CHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
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
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "3_Core.h"
#include "8_Renderer.h"

//=============================================================================
// OpenGL Core
//=============================================================================

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


	GLint activeAttribs = 0;
	GLint activeUniforms = 0;
	GLint activeBuffers = 0;
	GLint max0, max1;

	bool piqSupported = true
		&& GLAD_GL_ARB_program_interface_query
		&& GLAD_GL_ARB_shader_storage_buffer_object
		;

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
UniformVariable ShaderProgram::GetUniformVariable(const char* name)
{
	return { glGetUniformLocation(m_id, name) };
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, int value)
{
	glUniform1i(var.id, value);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, float value)
{
	glUniform1f(var.id, value);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, const glm::vec2& v)
{
	glUniform2fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, float x, float y, float z)
{
	glUniform3f(var.id, x, y, z);
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, const glm::vec3& v)
{
	glUniform3fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, const glm::vec4& v)
{
	glUniform4fv(var.id, 1, glm::value_ptr(v));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, const glm::mat3& mat)
{
	glUniformMatrix3fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}
//-----------------------------------------------------------------------------
void ShaderProgram::SetUniform(UniformVariable var, const glm::mat4& mat)
{
	glUniformMatrix4fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
}
//-----------------------------------------------------------------------------
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
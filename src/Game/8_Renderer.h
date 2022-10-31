#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

//=============================================================================
// ShaderProgram
//=============================================================================

struct UniformVariable
{
	UniformVariable() = default;
	UniformVariable(int newId) { id = newId; }
	bool IsValid() const { return id > -1; }
	int id = -1;
};

// TODO: юниформы хранящие свой тип данных (и статус изменения)
class ShaderProgram
{
public:
	bool CreateFromMemories(const std::string& vertexShaderMemory, const std::string& fragmentShaderMemory);
	void Destroy();

	void Bind();

	static void UnBind();

	UniformVariable GetUniformVariable(const char* name);

	void SetUniform(UniformVariable var, int value);
	void SetUniform(UniformVariable var, float value);
	void SetUniform(UniformVariable var, float x, float y, float z);
	void SetUniform(UniformVariable var, const glm::vec2& v);
	void SetUniform(UniformVariable var, const glm::vec3& v);
	void SetUniform(UniformVariable var, const glm::vec4& v);
	void SetUniform(UniformVariable var, const glm::mat3& mat);
	void SetUniform(UniformVariable var, const glm::mat4& mat);

	void SetUniform(const char* name, int value);
	void SetUniform(const char* name, float value);
	void SetUniform(const char* name, float x, float y, float z);
	void SetUniform(const char* name, const glm::vec2& v);
	void SetUniform(const char* name, const glm::vec3& v);
	void SetUniform(const char* name, const glm::vec4& v);
	void SetUniform(const char* name, const glm::mat3& mat);
	void SetUniform(const char* name, const glm::mat4& mat);

	bool IsValid() const { return m_id > 0; }

private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};
	unsigned createShader(ShaderType type, const std::string& source) const;

	unsigned m_id = 0;
};

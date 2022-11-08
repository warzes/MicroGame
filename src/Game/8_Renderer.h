#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

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

// TODO: юниформы хранящие свой тип данных (и статус изменения)
class ShaderProgram
{
public:
	bool CreateFromMemories(const std::string& vertexShaderMemory, const std::string& fragmentShaderMemory);
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

private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};
	unsigned createShader(ShaderType type, const std::string& source) const;

	unsigned m_id = 0;
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
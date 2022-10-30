#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

namespace scene
{
	class Transform
	{
	public:
		void Reset()
		{
			SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
			SetRotate(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		void SetPosition(const glm::vec3& pos) { m_translation = pos; m_update = true; }
		void SetScale(const glm::vec3& scale) { m_scale = scale; m_update = true; }
		void SetRotate(const glm::vec3& radianAngle) { m_quaternion = glm::quat(radianAngle); m_update = true; }

		const glm::mat4& GetWorldMatrix();
	private:
		glm::vec3 m_translation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::quat m_quaternion = glm::quat(glm::vec3(0.0f));
		glm::mat4 m_worldMatrix;
		bool m_update = true;
	};
}
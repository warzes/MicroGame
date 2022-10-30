#include "stdafx.h"
#include "13_Scene.h"


#pragma region Scene
namespace scene
{
	const glm::mat4& Transform::GetWorldMatrix()
	{
		if (m_update)
		{
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_translation);
			glm::mat4 rotationMatrix = glm::toMat4(m_quaternion);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);

			m_worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

			m_update = false;
		}
		return m_worldMatrix;
	}
}
#pragma endregion
#include "stdafx.h"
#include "13_Scene.h"
//-----------------------------------------------------------------------------
void Transform::updateTransforms()
{
	if (m_isTransformChanged & TransformChanged::NONE) return;

	// Calculate World Matrix
	m_worldMatrix = glm::translate(m_position) * glm::toMat4(m_rotation) * glm::scale(m_scale);


	if (m_parent)
		m_worldMatrix *= m_parent->m_worldMatrix; // TODO: делать дерево наследования, сейчас только одна ветвь

	// Get World Transform
	glm::vec3 pos, scale, skew; glm::vec4 perpective; glm::quat rot;
	if (glm::decompose(m_worldMatrix, scale, rot, pos, skew, perpective))
	{
		m_worldPosition = pos;
		m_worldScale = scale;
		m_worldRotation = rot;
	}

	m_forward = rot * glm::vec3(0.0f, 0.0f, 1.0f);
	m_right = rot * glm::vec3(1.0f, 0.0f, 0.0f);
	m_up = glm::cross(m_forward, m_right);

	m_isTransformChanged = TransformChanged::NONE;
}
//-----------------------------------------------------------------------------
void Transform::Translate(float x, float y, float z)
{
	Translate({ x, y, z });
}
//-----------------------------------------------------------------------------
void Transform::Translate(const glm::vec3& position)
{
	m_position = position;
	m_isTransformChanged |= TransformChanged::TRANSLATION;
}
//-----------------------------------------------------------------------------
void Transform::RotateEuler(float x, float y, float z)
{
	m_isTransformChanged |= TransformChanged::ROTATION;

	m_rotation = m_rotation * glm::quat_cast(glm::eulerAngleYXZ(y, x, z));
}
//-----------------------------------------------------------------------------
void Transform::RotateEuler(const glm::vec3& rotation)
{
	RotateEuler(rotation.x, rotation.y, rotation.z);
}
//-----------------------------------------------------------------------------
void Transform::Rotate(const glm::quat& rotation)
{
	m_isTransformChanged |= TransformChanged::ROTATION;
	m_rotation = m_rotation * rotation;
}
//-----------------------------------------------------------------------------
void Transform::Scale(float x, float y, float z)
{
	Scale({ x, y, z });
}
//-----------------------------------------------------------------------------
void Transform::Scale(const glm::vec3& scale)
{
	m_scale = scale;
	m_isTransformChanged |= TransformChanged::SCALE;
}
//-----------------------------------------------------------------------------
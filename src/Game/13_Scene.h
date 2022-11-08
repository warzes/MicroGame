#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

class Transform
{
public:
	void Translate(float x, float y, float z);
	void Translate(const glm::vec3& position);

	void RotateEuler(float x, float y, float z);
	void RotateEuler(const glm::vec3& rotation);
	void Rotate(const glm::quat& rotation);
	void SetRotation(const glm::quat& rotation) { m_isTransformChanged |= TransformChanged::ROTATION; m_rotation = rotation; }

	void Scale(float x, float y, float z);
	void Scale(const glm::vec3& scale);

	const glm::vec3& GetPosition() const { return m_position; }
	const glm::vec3& GetWorldPosition() const { return m_worldPosition; }
	const glm::vec3& GetScale() const { return m_scale; }
	const glm::vec3& GetWorldScale() const { return m_worldScale; }
	const glm::quat& GetRotation() const { return m_rotation; }
	const glm::vec3& GetYawPitchRoll() const { return glm::vec3(glm::yaw(m_rotation), glm::pitch(m_rotation), glm::roll(m_rotation)); };
	const glm::quat& GetWorldRotation() const { return m_worldRotation; }

	const glm::vec3& GetForward() const { return m_forward; }
	const glm::vec3& GetUp() const { return m_up; }
	const glm::vec3& GetRight() const { return m_right; }

	const glm::mat4& GetWorld() { updateTransforms(); return m_worldMatrix; }
private:
	void updateTransforms();

	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_worldPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_worldScale = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::quat m_rotation = glm::quat(glm::vec3(0.0f));
	glm::quat m_worldRotation = glm::quat(glm::vec3(0.0f));
	
	glm::mat4 m_worldMatrix = glm::mat4(1.0f);

	enum TransformChanged {
		NONE = 0x00,
		TRANSLATION = 0x01,
		ROTATION = 0x02,
		SCALE = 0x04,
	};
	uint8_t m_isTransformChanged = TransformChanged::TRANSLATION;

	Transform* m_parent = nullptr; // TODO:
};
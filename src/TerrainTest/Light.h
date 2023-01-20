#pragma once

namespace temp
{
	struct Light
	{
		glm::vec3 direction;
		glm::vec3 color;

		Light() = default;
		Light(glm::vec3 direction, glm::vec3 color)
		{
			this->direction = direction;
			this->color = color;
		}
	};
}
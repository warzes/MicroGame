#pragma once

namespace temp
{
	struct Texture
	{
		GLuint textureID;

		Texture() = default;
		Texture(GLuint textureID)
		{
			this->textureID = textureID;
		}
	};
}
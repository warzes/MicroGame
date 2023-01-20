#pragma once

namespace temp
{
	struct Model
	{
		GLuint vaoID;
		int vertexCount;

		Model() = default;
		Model(GLuint vaoID, int vertexCount)
		{
			this->vaoID = vaoID;
			this->vertexCount = vertexCount;
		}
	};
}
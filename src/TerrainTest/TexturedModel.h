#pragma once

namespace temp
{
	struct TexturedModel
	{
		Model model;
		Texture texture;

		TexturedModel() = default;
		TexturedModel(Model model, Texture texture)
		{
			this->model = model;
			this->texture = texture;
		}
	};
}
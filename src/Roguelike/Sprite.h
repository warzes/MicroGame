#pragma once

// позиции в игровом пространстве (а не пикселях)

class SpriteChar
{
public:
	static void Draw(const glm::vec2& pos, const glm::vec2& num, const glm::vec4& color);

	static void Flush();
};
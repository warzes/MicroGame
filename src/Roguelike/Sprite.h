#pragma once

// ������� � ������� ������������ (� �� ��������)

class SpriteChar
{
public:
	static void Init();
	static void Close();

	static void Draw(const glm::vec2& pos, const glm::vec2& num, const glm::vec4& color);

	static void Flush();
};
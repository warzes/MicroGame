#include "stdafx.h"
#include "DrawTextHelper.h"

g2d::Text CommonText;

void DrawTextHelper::DrawCommonText(const std::wstring str, const glm::vec2& pos, const glm::vec3& color)
{
	if (!CommonText.IsValid())
	{
		CommonText.Create("../data/fonts/OpenSans-Regular.ttf", 16);
	}

	const float widthHeight = ScreenHeight;
	const float widthScreen = widthHeight * GetFrameBufferAspectRatio();
	glm::mat4 ortho = glm::ortho(0.0f, widthScreen, widthHeight, 0.0f, 0.0f, 1.0f);
	CommonText.SetText(str);
	CommonText.Draw(glm::vec3(pos.x, pos.y, 0.1f), color, ortho);
}
#include "stdafx.h"
#include "DrawTextHelper.h"
#include "DrawHelper.h"

g2d::Text CommonText;

void DrawTextHelper::DrawCommonText(const std::wstring str, const glm::vec2& pos, const glm::vec3& color)
{
	if (!CommonText.IsValid())
	{
		CommonText.Create("../data/fonts/OpenSans-Regular.ttf", 16);
	}


	CommonText.SetText(str);
	CommonText.Draw(glm::vec3(pos.x, pos.y, 0.1f), color, DrawHelper::GetOrtho());
}
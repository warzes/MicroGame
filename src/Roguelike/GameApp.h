﻿#pragma once

#include "Sprite.h"
#include "DrawHelper.h"
#include "World.h"

#include "Character.h"
#include "MinimapRender.h"
//-----------------------------------------------------------------------------
механика боя такая
бой происходит в режиме боя. игра переходит в режим боя при контакте с монстром (т оесть игрок и монстр стоят рядом на соседних клетках).при этом не важно кто первым коснулся врага, очередность определяется ролевой системой
ход игрока и ход монстра действует также как и в режиме путешествия, а значит могут подойти союзники.
в свой ход игрок совершает одно действие - для этого в его ход появляется меню где можно:
- удар - простой удар, дает 1 очко средоточия
- блок/паррирование - позволяет снизить урон при успехе, дает 2 очка средоточия, при прокачанном навыке может вызвать контрудар
- уклонение - при успехе позволяет полностью проигнорировать урон, дает 2 очка средоточия
- таланты - позволяет использовать мощные удары тратя накопленные очки средоточия
- заклинания (только ближнего боя или мгновенные)
- инвентарь - позволяет выпить зелье, заюзать свиток или бросить предмет
- отступление - при успехе позволяет выйти из боя (монстр на пару ходов становится серым)

Стрельба - действует только в режиме иследования, так как в бою сложно стрелять.

World world;
MinimapRender minimapRender;
//-----------------------------------------------------------------------------
bool StartGameApp()
{
	if (!minimapRender.Create())
		return false;

	RenderSystem::SetFrameColor(glm::vec3(0.15, 0.15, 0.15));

	SpriteChar::Init();

	world.SetMap(L"test");

	return true;
}
//-----------------------------------------------------------------------------
void CloseGameApp()
{
	SpriteChar::Close();
	minimapRender.Destroy();
}
//-----------------------------------------------------------------------------
void UpdateGameApp(float deltaTime)
{
	world.UpdatePlayer(deltaTime);
}
//-----------------------------------------------------------------------------
void FrameGameApp(float deltaTime)
{
	DrawHelper::DrawMainUI();
	world.Draw();

	SpriteChar::Flush();
	minimapRender.Draw(world);
}
//-----------------------------------------------------------------------------
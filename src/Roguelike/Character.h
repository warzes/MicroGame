#pragma once

class CharacterInfo
{
public:
	std::string name;

	// attribute 
	// (0), bonus (1)
	int stratt[2]; // Strength
	int staatt[2]; // Stamina
	int dexatt[2]; // Dexterity
	int aglatt[2]; // Agility
	int intatt[2]; // Intelligence

	int currhp; // Current HP
	int maxhp; // Max HP

	// skill
	// (0), bonus (1)
	int ucfsk[2]; // Unarmed combat skill
	int acfsk[2]; // Armed combat skill
	int pcfsk[2]; // Projectile combat skill
	int mcfsk[2]; // Magic combat skill
	int cdfsk[2]; // Combat defense skill
	int mdfsk[2]; // Magic defense skill

	int currxp; // Current spendable XP amount.
	int totxp; // Lifetime XP amount.0
	int currgold; // Current gold amount.
	int totgold; // Lifetime gold amount.

	int locx; // Current x position on map.
	int locy; // Current y location on map.
};

class Character
{
public:
	CharacterInfo cinfo;
};
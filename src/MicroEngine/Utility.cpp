#include "stdafx.h"
#include "Utility.h"
//-----------------------------------------------------------------------------
Rng::Rng(unsigned int seed)
	: m_seed(seed)
	, m_mt(seed)
{
	//std::cout << "RNG Seed: 0x" << std::hex << seed << std::dec << '\n';
}
//-----------------------------------------------------------------------------
void Rng::Reset()
{
	SetSeed(std::random_device()());
}
//-----------------------------------------------------------------------------
void Rng::SetSeed(unsigned int seed)
{
	m_seed = seed;
	m_mt.seed(seed);

	//std::cout << "RNG Seed: 0x" << std::hex << seed << std::dec << '\n';
}
//-----------------------------------------------------------------------------
unsigned int Rng::GetSeed() const
{
	return m_seed;
}
//-----------------------------------------------------------------------------
int Rng::GetInt(int exclusiveMax)
{
	return std::uniform_int_distribution<>(0, exclusiveMax - 1)(m_mt);
}
//-----------------------------------------------------------------------------
int Rng::GetInt(int min, int inclusiveMax)
{
	return min + std::uniform_int_distribution<>(0, inclusiveMax - min)(m_mt);
}
//-----------------------------------------------------------------------------
bool Rng::GetBool(double probability)
{
	return std::bernoulli_distribution(probability)(m_mt);
}
//-----------------------------------------------------------------------------
float Rng::GetFloat(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(m_mt);
}
//-----------------------------------------------------------------------------
int Rng::RollDice(int n, int s)
{
	int result = 0;

	for (int i = 0; i < n; ++i)
		result += GetInt(1, s);

	return result;
}
//-----------------------------------------------------------------------------
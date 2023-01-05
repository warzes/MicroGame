#pragma once

#include "BaseHeader.h"

// Random Number Generator
class Rng
{
public:
	explicit Rng(unsigned int seed = std::random_device()());

	void Reset();
	void SetSeed(unsigned int seed);
	unsigned int GetSeed() const;

	int GetInt(int exclusiveMax); // [0, max)
	int GetInt(int min, int inclusiveMax); // [min, max]

	bool GetBool(double probability = 0.5);
	float GetFloat(float min, float max); // [min, max)

	int RollDice(int n, int s); // roll S sided dice N times

	template <typename T>
	const T& GetOne(const std::vector<T>& vector);

	template <typename T>
	void Shuffle(std::vector<T>& vector);

private:
	unsigned int m_seed;
	std::mt19937 m_mt;
};

template <typename T>
const T& Rng::GetOne(const std::vector<T>& vector)
{
	return vector[GetInt(vector.size())];
}

template <typename T>
void Rng::Shuffle(std::vector<T>& vector)
{
	std::shuffle(vector.begin(), vector.end(), m_mt);
}
#pragma once

#include <random>
#include <chrono>
#include "CompilerSettings.h"

class Random
{
public:

	// Returns a uniform integral distribution, evenly distributed across all possible values of T
	template <typename T>
	CMPINLINE static std::uniform_int_distribution<T> UniformIntegralDistribution(void)
	{
		return std::uniform_int_distribution<T>();
	}

	// Returns a uniform integral distribution between the given values
	template <typename T>
	CMPINLINE static std::uniform_int_distribution<T> UniformIntegralDistribution(T _min, T _max)
	{
		return std::uniform_int_distribution<T>(_min, _max);
	}

	// Returns a uniform real distribution, evenly distributed across all possible values of T
	template <typename T>
	CMPINLINE static std::uniform_real_distribution<T> UniformRealDistribution(void)
	{
		return std::uniform_real_distribution<T>();
	}
	// Returns a uniform real distribution between the given values
	template <typename T>
	CMPINLINE static std::uniform_real_distribution<T> UniformRealDistribution(T _min, T _max)
	{
		return std::uniform_real_distribution<T>(_min, _max);
	}

	// Generates a new random integral based on the given uniform distribution
	template <typename T>
	CMPINLINE static T GetUniformIntegral(std::uniform_int_distribution<T> distribution)
	{
		return distribution(m_generator);
	}

	// Generates a new random real based on the given uniform distribution
	template <typename T>
	CMPINLINE static T GetUniformReal(std::uniform_real_distribution<T> distribution)
	{
		return distribution(m_generator);
	}


private:

	static std::default_random_engine				m_generator;

};

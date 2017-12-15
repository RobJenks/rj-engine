#pragma once

#include <cassert>
#include <DirectXMath.h>
#include "CompilerSettings.h"
#define DEFAULT_VAL 0.0f


struct DefaultComponent
{
	struct USE_DEFAULT;
	struct NO_DEFAULT;
};

template <typename TDefaultX, typename TDefaultY, typename TDefaultZ, typename TDefaultW>
class DefaultingFloat4
{
public:

	float x, y, z, w;

	CMPINLINE DefaultingFloat4(float _x, float _y, float _z, float _w) noexcept
		:
		x(_x), y(_y), z(_z), w(_w) 
	{ }

	CMPINLINE DefaultingFloat4(const DirectX::XMFLOAT4 & vec) noexcept
		:
		x(vec.x), y(vec.y), z(vec.z), w(vec.w)
	{ }

	CMPINLINE DefaultingFloat4(void) noexcept
	{
		// Default no-args constructor can only exist in a fully-specialised state
		static_assert(false, "DefaultingFloat4 must be parameterised");
	}

	// Copy constructor
	CMPINLINE DefaultingFloat4(const DefaultingFloat4 & other) noexcept
		:
		x(other.x), y(other.y), z(other.z), w(other.w)
	{
	}

	// Copy assignment
	CMPINLINE DefaultingFloat4 & DefaultingFloat4::operator=(const DefaultingFloat4 & other) noexcept
		:
		x(other.x), y(other.y), z(other.z), w(other.w)
	{
	}


	// Move constructor
	CMPINLINE DefaultingFloat4(DefaultingFloat4 && other) noexcept
		:
		x(other.x), y(other.y), z(other.z), w(other.w)
	{
	}


	// Move assignment
	CMPINLINE DefaultingFloat4 & DefaultingFloat4::operator=(DefaultingFloat4 && other) noexcept
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}
};

// Explicit default constructor specialisations to allow compile-time specification of defaulted parameters
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	{}
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: y(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: z(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), y(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), z(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: y(DEFAULT_VAL), z(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: y(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: z(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), y(DEFAULT_VAL), z(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), y(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: y(DEFAULT_VAL), z(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), z(DEFAULT_VAL), w(DEFAULT_VAL) { }
template <> DefaultingFloat4<DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT, DefaultComponent::USE_DEFAULT>::DefaultingFloat4(void) noexcept
	: x(DEFAULT_VAL), y(DEFAULT_VAL), z(DEFAULT_VAL), w(DEFAULT_VAL) { }






#pragma once

#ifndef __RayH__
#define __RayH__

#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Ray : public ALIGN16<Ray>
{
public:

	// Member fields
	AXMVECTOR				Origin;					// Specified in ray definition
	AXMVECTOR				Direction;				// Specified in ray definition
	AXMVECTOR				InvDirection;			// Derived upon ray construction
	AXMVECTOR				Sign;					// Derived upon ray construction; holds -1 or +1 depending on sign of inverse direction vector

	// Constructors
	Ray(void) { }
	Ray(const FXMVECTOR origin, const FXMVECTOR direction)
	{
		SetOrigin(origin);
		SetDirection(direction);
	}

	// Construct a Ray from a BasicRay object
	Ray(const BasicRay & basic_ray) : Ray(basic_ray.Origin, basic_ray.Direction) { }

	// Copy constructor
	Ray(const Ray &r)
	{
		Origin = r.Origin;
		Direction = r.Direction;
		InvDirection = r.InvDirection;
		Sign = r.Sign;
	}

	// Transforms the ray into a different coordinate system
	void TransformIntoCoordinateSystem(const FXMVECTOR origin, const AXMVECTOR_P(&bases)[3]);

	// Get a position along the ray at the specified time
	CMPINLINE XMVECTOR			PositionAtTime(float t) const
	{
		return XMVectorMultiplyAdd(Direction, XMVectorReplicate(t), Origin);
	}

	// Get a position along the ray at the specified (vectorised) time
	CMPINLINE XMVECTOR			PositionAtTime(const FXMVECTOR t) const
	{
		return XMVectorMultiplyAdd(Direction, t, Origin);
	}

	// Update the ray origin
	CMPINLINE void				SetOrigin(const FXMVECTOR origin)
	{
		Origin = origin;
	}

	// Move the ray origin
	CMPINLINE void				MoveOrigin(const FXMVECTOR offset)
	{
		Origin = XMVectorAdd(Origin, offset);
	}

	// Update the ray direction
	void						SetDirection(const FXMVECTOR direction);

	// Generates a string representation of the ray
	std::string					ToString(void) const;


};



#endif
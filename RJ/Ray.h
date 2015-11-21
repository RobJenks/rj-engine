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
		: Origin(origin), Direction(direction)
	{
		InvDirection = XMVectorReciprocal(Direction);
		Sign = XMVectorSelect(ONE_VECTOR_P, ONE_VECTOR_N, XMVectorLess(InvDirection, ZERO_VECTOR));
	}

	// Copy constructor
	Ray(const Ray &r)
	{
		Origin = r.Origin;
		Direction = r.Direction;
		InvDirection = r.InvDirection;
		Sign = r.Sign;
	}

	// Transforms the ray into a different coordinate system
	void TransformIntoCoordinateSystem(const FXMVECTOR origin, const XMVECTOR(&bases)[3])
	{
		XMVECTOR diff = XMVectorSubtract(Origin, origin);
		
		// Transform the ray origin
		Origin = XMVectorSet(	XMVectorGetX(XMVector3Dot(diff, bases[0])),
								XMVectorGetX(XMVector3Dot(diff, bases[1])),
								XMVectorGetX(XMVector3Dot(diff, bases[2])), 0.0f);

		// Transform the ray direction
		Direction = XMVectorSet(XMVectorGetX(XMVector3Dot(Direction, bases[0])),
								XMVectorGetX(XMVector3Dot(Direction, bases[1])),
								XMVectorGetX(XMVector3Dot(Direction, bases[2])), 0.0f);
			
		// Recalculate derived fields
		InvDirection = XMVectorReciprocal(Direction);
		Sign = XMVectorSelect(ONE_VECTOR_P, ONE_VECTOR_N, XMVectorLess(InvDirection, ZERO_VECTOR));
	}

};



#endif
#pragma once

#ifndef __AABBH__
#define __AABBH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "OrientedBoundingBox.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class AABB : public ALIGN16<AABB>
{
public:
	
	// AABB can be defined using only two points; P0 = min point, P1 = max point
	AXMVECTOR					P0;
	AXMVECTOR					P1;

	// Convenience methods to get/set the min and max AABB points
	const XMVECTOR &			MinPoint(void) const			{ return P0; }
	const XMVECTOR &			MaxPoint(void) const			{ return P1; }
	void						SetMin(const FXMVECTOR v)		{ P0 = v; }
	void						SetMax(const FXMVECTOR v)		{ P1 = v; }

	// Default constructor
	AABB(void) { }

	// Constructor that builds an AABB based upon its minimum and maximum points
	AABB(const FXMVECTOR vmin, const FXMVECTOR vmax) 
		:  P0(vmin), P1(vmax)
	{
	}

	// Constructor to build an AABB from an OBB.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	AABB(const OrientedBoundingBox & obb)
	{
		P1 = obb.Data.ExtentV;
		P0 = XMVectorNegate(P1);
	}

	// Constructor to build an AABB from OBB core data.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	AABB(const OrientedBoundingBox::CoreOBBData & obb)
	{
		P1 = obb.ExtentV;
		P0 = XMVectorNegate(P1);
	}
};



#endif
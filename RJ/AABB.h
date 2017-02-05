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
	AXMVECTOR							P0;
	AXMVECTOR							P1;

	// Convenience methods to get/set the min and max AABB points
	CMPINLINE const XMVECTOR &			MinPoint(void) const			{ return P0; }
	CMPINLINE const XMVECTOR &			MaxPoint(void) const			{ return P1; }
	CMPINLINE void						SetMin(const FXMVECTOR v)		{ P0 = v; }
	CMPINLINE void						SetMax(const FXMVECTOR v)		{ P1 = v; }

	// Default constructor
	CMPINLINE AABB(void) { }

	// Constructor that builds an AABB based upon its minimum and maximum points
	CMPINLINE AABB(const FXMVECTOR vmin, const FXMVECTOR vmax)
		:  P0(vmin), P1(vmax)
	{
	}

	// Constructor to build an AABB from an OBB.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	CMPINLINE AABB(const OrientedBoundingBox & obb)
	{
		P1 = obb.ConstData().ExtentV;
		P0 = XMVectorNegate(P1);
	}

	// Constructor to build an AABB from OBB core data.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	CMPINLINE AABB(const OrientedBoundingBox::CoreOBBData & obb)
	{
		P1 = obb.ExtentV;
		P0 = XMVectorNegate(P1);
	}

	// Constructor to build an AABB from OBB core data.  Accepts a parameter used to expand/contract the AABB 
	// bounds (centre-to-edge, i.e. half of overall size) by a specified amount on construction
	CMPINLINE AABB(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR bounds_adjustment)
	{
		P1 = XMVectorAdd(obb.ExtentV, bounds_adjustment);
		P0 = XMVectorNegate(P1);
	}

	// String representation of the AABB
	CMPINLINE std::string str(void) const
	{
		return concat("AABB { min=")(Vector3ToString(P0))(", max=")(Vector3ToString(P1))(" }").str();
	}
};



#endif
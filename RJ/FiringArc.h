#pragma once

#ifndef __FiringArcH__
#define __FiringArcH__

#include "Arc2D.h"

// Represents a unit arc circle centred at (0,0), for the purposes of representing a firing arc.  
// Wrapper around the Arc2D class
// This class DOES NOT have any special alignment requirements
class FiringArc 
{
public:

	// The arc data 
	Arc2D ArcData;

	// Default constructor; does nothing
	FiringArc(void) { }

	// Constructor; accepts the minimum and maximum angles that define the arc
	FiringArc(float arc_begin_angle, float arc_end_angle)
		:
		ArcData(NULL_FLOAT2, 1.0f, arc_begin_angle, arc_end_angle)
	{
	}

	// Determines whether the given point, or equivalently the vector from arc origin (0,0), is within 
	// the arc extents.  'vec2' is a point in local firing arc space, i.e. (0,1) is due north
	CMPINLINE bool VectorWithinArc(FXMVECTOR vec2) const 
	{
		// Normalise the point to ensure it lies on the unit firing arc circle, then test against the arc data and return
		return ArcData.ContainsPoint(XMVector2NormalizeEst(vec2));
	}

	// Determines whether the given point, or equivalently the vector from arc origin (0,0), is within 
	// the arc extents.  'pt_norm' is a point in local firing arc space, i.e. (0,1) is due north.  'vec2_norm'
	// must be a normalised vector otherwise behaviour is undefined
	CMPINLINE bool UnitVectorWithinArc(const FXMVECTOR vec2_norm) const
	{
		// Test against the arc data and return
		return ArcData.ContainsPoint(vec2_norm);
	}

	// Determines whether the given point, or equivalently the vector from arc origin (0,0), is within 
	// the arc extents.  'pt_norm' is a point in local firing arc space, i.e. (0,1) is due north.  'vec2_norm'
	// must be a normalised vector otherwise behaviour is undefined
	CMPINLINE bool UnitVectorWithinArc(const XMFLOAT2 & vec2_norm) const
	{
		// Test against the arc data and return
		return ArcData.ContainsPoint(vec2_norm);
	}

};



#endif
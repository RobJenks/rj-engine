#pragma once

#ifndef __FiringArcH__
#define __FiringArcH__

#include "Arc2D.h"

// Represents a unit arc circle centred at (0,0), for the purposes of representing a firing arc.  
// Wrapper around the Arc2D class
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
		ArcData(NULL_VECTOR2, 1.0f, arc_begin_angle, arc_end_angle)
	{
	}

	// Determines whether the given point, or equivalently the vector from arc origin (0,0), is within 
	// the arc extents.  'pt' is a point in local firing arc space, i.e. (0,1) is due north
	CMPINLINE bool VectorWithinArc(D3DXVECTOR2 pt) const 
	{
		// Normalise the point to ensure it lies on the unit firing arc circle
		D3DXVec2Normalize(&pt, &pt);

		// Test against the arc data and return
		return ArcData.ContainsPoint(pt);
	}

	// Determines whether the given point, or equivalently the vector from arc origin (0,0), is within 
	// the arc extents.  'pt_norm' is a point in local firing arc space, i.e. (0,1) is due north.  'pt_norm'
	// must be a normalised vector otherwise behaviour is undefined
	CMPINLINE bool UnitVectorWithinArc(const D3DXVECTOR2 & pt_norm) const
	{
		// Test against the arc data and return
		return ArcData.ContainsPoint(pt_norm);
	}

};



#endif
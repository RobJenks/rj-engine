#pragma once

#ifndef __AABBH__
#define __AABBH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "OrientedBoundingBox.h"

class AABB
{
public:
	
	// AABB can be defined using only two points; Points[0] = min point, Points[1] = max point
	D3DXVECTOR3					Points[2];

	// Convenience methods to get/set the min and max AABB points
	const D3DXVECTOR3 &			MinPoint(void) const			{ return Points[0]; }
	const D3DXVECTOR3 &			MaxPoint(void) const			{ return Points[1]; }
	void						SetMin(const D3DXVECTOR3 & v)	{ Points[0] = v; }
	void						SetMax(const D3DXVECTOR3 & v)	{ Points[1] = v; }

	// Default constructor
	AABB(void) { }

	// Constructor that builds an AABB based upon its minimum and maximum points
	AABB(const D3DXVECTOR3 & vmin, const D3DXVECTOR3 & vmax)
	{
		Points[0] = vmin; Points[1] = vmax;
	}

	// Constructor to build an AABB from an OBB.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	AABB(const OrientedBoundingBox & obb)
	{
		Points[1] = obb.Data.Extent;
		Points[0] = -Points[1];
	}

	// Constructor to build an AABB from OBB core data.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	AABB(const OrientedBoundingBox::CoreOBBData & obb)
	{
		Points[1] = obb.Extent;
		Points[0] = -Points[1];
	}
};



#endif
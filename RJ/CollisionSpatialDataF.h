#pragma once

#include "DX11_Core.h"
#include "FastMath.h"

struct CollisionSpatialDataF
{
	XMFLOAT3			Position;
	XMFLOAT4			Orientation;
	XMFLOAT3			Extent;

	// Constructor
	CollisionSpatialDataF(void) : Position(NULL_FLOAT3), Orientation(ID_QUATERNIONF), Extent(NULL_FLOAT3) { }

	// Constructor
	CollisionSpatialDataF(const XMFLOAT3 & position, const XMFLOAT4 & orient, const XMFLOAT3 & extent) 
		: Position(position), Orientation(orient), Extent(extent) { }
};

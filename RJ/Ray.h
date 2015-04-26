#pragma once

#ifndef __RayH__
#define __RayH__

#include "DX11_Core.h"


class Ray
{
public:

	// Member fields
	D3DXVECTOR3				Origin;					// Specified in ray definition
	D3DXVECTOR3				Direction;				// Specified in ray definition
	D3DXVECTOR3				InvDirection;			// Derived upon ray construction
	int						Sign[3];				// Derived upon ray construction

	// Constructors
	Ray(void) { }
	Ray(const D3DXVECTOR3 & origin, const D3DXVECTOR3 & direction)
		: Origin(origin), Direction(direction)
	{
		InvDirection = D3DXVECTOR3(1.0f / Direction.x, 1.0f / Direction.y, 1.0f / Direction.z);
		Sign[0] = InvDirection.x < 0.0f;
		Sign[1] = InvDirection.y < 0.0f;
		Sign[2] = InvDirection.z < 0.0f;
	}

	// Copy constructor
	Ray(const Ray &r)
	{
		Origin = r.Origin;
		Direction = r.Direction;
		InvDirection = r.InvDirection;
		Sign[0] = r.Sign[0]; Sign[1] = r.Sign[1]; Sign[2] = r.Sign[2];
	}

	// Transforms the ray into a different coordinate system
	void TransformIntoCoordinateSystem(const D3DXVECTOR3 & origin, const D3DXVECTOR3(&bases)[3])
	{
		D3DXVECTOR3 diff = (Origin - origin);

		// Transform the ray origin
		Origin = D3DXVECTOR3(
			D3DXVec3Dot(&diff, &(bases[0])),
			D3DXVec3Dot(&diff, &(bases[1])),
			D3DXVec3Dot(&diff, &(bases[2]))
		);

		// Transform the ray direction
		Direction = D3DXVECTOR3(
			D3DXVec3Dot(&Direction, &(bases[0])),
			D3DXVec3Dot(&Direction, &(bases[1])),
			D3DXVec3Dot(&Direction, &(bases[2]))
		);

		// Recalculate derived fields
		InvDirection = D3DXVECTOR3(1.0f / Direction.x, 1.0f / Direction.y, 1.0f / Direction.z);
		Sign[0] = InvDirection.x < 0.0f;
		Sign[1] = InvDirection.y < 0.0f;
		Sign[2] = InvDirection.z < 0.0f;
	}

};



#endif
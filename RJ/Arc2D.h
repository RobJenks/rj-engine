#pragma once

#ifndef __Arc2DH__
#define __Arc2DH__

#include "FastMath.h"
#include "DX11_Core.h"

class Arc2D
{
public:

	D3DXVECTOR2					Centre;				// Centre point of the circle of which this arc is a part
	float						Radius;				// The radius of the arc circle
	D3DXVECTOR2					ArcPoints[2];		// Two points lying on the circle circumference that define the arc
													// extent.  Both points must lie on the circumference.  pt[1]
													// should be COUNTERclockwise from pt[0] on the circle

protected:
	D3DXVECTOR2					ArcPointDiff;		// The vector difference of AP0/1, i.e. (ArcPoints[1] - ArcPoints[0])


public:

	// Recalculates derived fields based on primary data.  Should be called if the arc parameters are changed directly
	void Recalculate(void)
	{
		ArcPointDiff = (ArcPoints[1] - ArcPoints[0]);
	}


	// Default constructor; does nothing
	Arc2D(void) { }

	// Contructor; accepts the circle centre/radius and the two arc points on its circumference
	Arc2D(const D3DXVECTOR2 & centre, float radius, const D3DXVECTOR2 & pt0, const D3DXVECTOR2 & pt1)
		:
		Centre(centre),
		Radius(radius)
	{
		ArcPoints[0] = pt0;
		ArcPoints[1] = pt1;

		// Recalc derived data based on these values
		Recalculate();
	}

	// Constructor; accepts the circle centre/radius, and the angle (radians) around the circle at 
	// which the arc begins and ends.  Calculates arc points using trigonometric functions
	Arc2D(const D3DXVECTOR2 & centre, float radius, float arc_begin_angle, float arc_end_angle)
		:
		Centre(centre),
		Radius(radius)
	{
		// Scale the arc angles to lie in the range (0 2PI]
		arc_begin_angle = fmod(arc_begin_angle, TWOPI);
		arc_end_angle = fmod(arc_end_angle, TWOPI);

		// Determine the location of each point using trig; given the hypotenuse (radius) and angle, 
		// we can derive the x & y coords of P as opposite & adjacent edges of the triangle CP[X]
		// pt[1] should be counterclockwise, i.e. lie before, pt[0] on the circle.  We therefore
		// use the begin_angle to derive pt[1] and the end_angle to derive pt[0]
		ArcPoints[1] = D3DXVECTOR2(radius * sinf(arc_begin_angle), radius * cos(arc_begin_angle));
		ArcPoints[0] = D3DXVECTOR2(radius * sinf(arc_end_angle), radius * cos(arc_end_angle));

		// Recalc derived data based on these values
		Recalculate();
	}

	// Determines whether the given point lies within the arc.  Point must lie on the circumference
	// of the arc circle (i.e. the vector CP must have magnitude == 'radius')
	bool ContainsPoint(const D3DXVECTOR2 & pt) const
	{
		// Assert: |P-C| = R where P is the input point, C is the circle center,
		// and R is the circle radius.  For P to be on the arc from A to B, it
		// must be on the side of the plane containing A with normal N = Perp(B-A)
		// where Perp(u,v) = (v,-u).  (From GTEngine 'GteArc2')
		D3DXVECTOR2 diffPtA0 = (pt - ArcPoints[0]);
		return ( DOTPERP_2D(diffPtA0, ArcPointDiff) >= 0.0f );
	}
};



#endif

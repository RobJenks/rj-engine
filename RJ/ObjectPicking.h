#pragma once

#ifndef __ObjectPickingH__
#define __ObjectPickingH__

#include "Utility.h"
#include "BasicRay.h"
class iObject;

// Class has no special alignment requirements
class ObjectPicking
{
public:

	// Method which generates a world-space ray based on the specified, normalised screen location
	static void							ScreenSpaceToWorldBasicRay(const XMFLOAT2 & norm_screen_location, BasicRay & outRay);
	
	// Method which generates a world-space ray based on the specified, normalised screen location
	static void							ScreenSpaceToWorldRay(const XMFLOAT2 & norm_screen_location, Ray & outRay);

	// Returns the object being selected at the specified mouse location, or NULL if none.  Expects
	// normalised mouse location as input
	static iObject *					GetObjectAtMouseLocation(const XMFLOAT2 & norm_mouse_location);

	// Returns the object being selected at the current mouse location, or NULL if none.  Uses the precalculated
	// mouse world ray and so is more efficient than overloads if we want to use the current mouse location
	static iObject *					GetObjectAtMouseLocation(void);


	// Returns the object being intersected by the specified mouse world ray, or NULL if none
	static iObject *					GetObjectIntersectedByMouseWorldRay(const BasicRay & ray);
};

#endif
#pragma once

#ifndef __ConstraintH__
#define __ConstraintH__

#include "DX11_Core.h"
#include "CompilerSettings.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Constraint : public ALIGN16<Constraint>
{
public:

	// Constructor; create a new constraint object
	Constraint(const FXMVECTOR parent_point, const FXMVECTOR axis);
	
	// Retrieve the axis of rotation (in parent object space) for this constraint
	CMPINLINE const XMVECTOR & 					GetAxis(void) const						{ return m_axis; }

	// Retrieve the point on the parent object which the constraint axis passes through
	CMPINLINE const XMVECTOR & 					GetParentPoint(void) const				{ return m_parent_point; }

	// Retrieve the point on the child object which the constraint axis passes through
	CMPINLINE const XMVECTOR & 					GetChildPoint(void) const				{ return m_parent_point; }

	// Retrieve the base orientation of the parent object, relative to the child, i.e. the initial state when the constraint was assigned
	CMPINLINE const XMVECTOR &					GetBaseParentOrientation(void) const	{ return m_base_parent_orient; }

	// Retrieve the base orientation of the child object, relative to the parent, i.e. the initial state when the constraint was assigned
	CMPINLINE const XMVECTOR &					GetBaseChildOrientation(void) const		{ return m_base_parent_orient; }

	// Calculates the new orientation of the child relative to its parent, based on a child rotation about the constraint
	XMVECTOR									CalculateChildRotationAboutConstraint(float radians);

	// Calculates the new orientation of the parent relative to its child, based on a parent rotation about the constraint
	XMVECTOR									CalculateParentRotationAboutConstraint(float radians);

protected:

	AXMVECTOR										m_axis;						// Axis of rotation, in parent object space
	AXMVECTOR										m_parent_point;				// Point on the parent object which the axis passes through

	AXMVECTOR										m_child_point;				// Point on the parent object which the axis passes through
	AXMVECTOR										m_base_parent_orient;		// Base orientation of the parent object, relative to the child
	AXMVECTOR										m_base_child_orient;		// Base orientation of the child object, relative to the parent
		
};












#endif
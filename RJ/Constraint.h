#pragma once

#ifndef __ConstraintH__
#define __ConstraintH__

#include "DX11_Core.h"
#include "CompilerSettings.h"

class Constraint
{
public:

	// Constructor; create a new constraint object
	Constraint(const D3DXVECTOR3 & parent_point, const D3DXVECTOR3 & axis);
	
	// Retrieve the axis of rotation (in parent object space) for this constraint
	CMPINLINE const D3DXVECTOR3 & 					GetAxis(void) const						{ return m_axis; }

	// Retrieve the point on the parent object which the constraint axis passes through
	CMPINLINE const D3DXVECTOR3 & 					GetParentPoint(void) const				{ return m_parent_point; }

	// Retrieve the point on the child object which the constraint axis passes through
	CMPINLINE const D3DXVECTOR3 & 					GetChildPoint(void) const				{ return m_parent_point; }

	// Retrieve the base orientation of the parent object, relative to the child, i.e. the initial state when the constraint was assigned
	CMPINLINE const D3DXQUATERNION &				GetBaseParentOrientation(void) const	{ return m_base_parent_orient; }

	// Retrieve the base orientation of the child object, relative to the parent, i.e. the initial state when the constraint was assigned
	CMPINLINE const D3DXQUATERNION &				GetBaseChildOrientation(void) const		{ return m_base_parent_orient; }

	// Calculates the new orientation of the child relative to its parent, based on a child rotation about the constraint
	D3DXQUATERNION									CalculateChildRotationAboutConstraint(float radians);

	// Calculates the new orientation of the parent relative to its child, based on a parent rotation about the constraint
	D3DXQUATERNION									CalculateParentRotationAboutConstraint(float radians);

protected:

	D3DXVECTOR3										m_axis;						// Axis of rotation, in parent object space
	D3DXVECTOR3										m_parent_point;				// Point on the parent object which the axis passes through

	D3DXVECTOR3										m_child_point;				// Point on the parent object which the axis passes through
	D3DXQUATERNION									m_base_parent_orient;		// Base orientation of the parent object, relative to the child
	D3DXQUATERNION									m_base_child_orient;		// Base orientation of the child object, relative to the parent


};












#endif
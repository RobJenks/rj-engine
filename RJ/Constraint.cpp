#include "Constraint.h"


// Constructor; create a new constraint object
Constraint::Constraint(const D3DXVECTOR3 & parent_point, const D3DXVECTOR3 & axis)
	:
	m_parent_point(parent_point), m_axis(axis)
{
}

// Calculates the new orientation of the child relative to its parent, based on a child rotation about the constraint
D3DXQUATERNION									CalculateChildRotationAboutConstraint(float radians);

// Calculates the new orientation of the parent relative to its child, based on a parent rotation about the constraint
D3DXQUATERNION									CalculateParentRotationAboutConstraint(float radians);
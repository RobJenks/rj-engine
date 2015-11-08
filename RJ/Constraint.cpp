#include "Constraint.h"


// Constructor; create a new constraint object
Constraint::Constraint(const FXMVECTOR parent_point, const FXMVECTOR axis)
	:
	m_parent_point(parent_point), m_axis(axis)
{
}

// Calculates the new orientation of the child relative to its parent, based on a child rotation about the constraint
XMVECTOR									CalculateChildRotationAboutConstraint(float radians);

// Calculates the new orientation of the parent relative to its child, based on a parent rotation about the constraint
XMVECTOR									CalculateParentRotationAboutConstraint(float radians);
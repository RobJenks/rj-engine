#pragma once

#ifndef __AttachmentH__
#define __AttachmentH__

#include "DX11_Core.h"
#include "CompilerSettings.h"


class _Attachment_Internal
{
public:
	// Struct holding temporary fields for intermediate calculations
	static struct _DATA_STRUCT { D3DXMATRIX m1, m2, m3, m4; D3DXVECTOR3 v1, v2, v3; D3DXQUATERNION q1, q2, q3; } _DATA;
};

// Struct representing a constraint between the two objects, for non-static attachments
struct AttachmentConstraint
{
	D3DXVECTOR3										Axis;					// Axis of rotation, in parent object space
	float											Rotation;				// Angle of rotation about the axis, in radians, from base orientation

	D3DXVECTOR3										ParentPoint;			// Point on the parent object which the axis passes through
	D3DXVECTOR3										ChildPoint;				// Point on the child object which the axis passes through
	D3DXMATRIX										InvChildPointOffset;	// Cached translation matrix to (-ChildPoint)
	
	D3DXQUATERNION									BaseChildOrient;		// Base orientation of the child object, relative to the parent
	D3DXQUATERNION									BaseParentOrient;		// Base orientation of the parent object, relative to the 
																			// child.  Is Inverse(BaseChildOrient)

	// Default constructor to create new constraint data; no initialisation performed here
	AttachmentConstraint(void) { }
};

template <typename T>
class Attachment
{

public:
	T								Parent;					// Parent object
	T								Child;					// Child object

	AttachmentConstraint *			Constraint;				// Constraint between the two objects, for non-static attachments


	// Position offset of the child from the parent
	CMPINLINE D3DXVECTOR3			GetPositionOffset(void) const						{ return m_posoffset; }
	void							SetPositionOffset(const D3DXVECTOR3 & off);

	// Orientation offset of the child from the parent
	CMPINLINE D3DXQUATERNION		GetOrientationOffset(void) const					{ return m_orientoffset; }
	void							SetOrientationOffset(const D3DXQUATERNION & off);

	// Set both offsets at once, for efficiency
	void							SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff);

	// Update the parent or child object
	void							SetParent(T parent)									{ Parent = parent; }
	void							SetChild(T child)									{ Child = child; }


	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// the object world matrix and stores its new position, and is relatively fast.  Use when orientation is not required	
	void							Apply_PositionOnly(void)
	{
		// Update the child object position based on the parent world matrix
		D3DXVec3TransformCoord(&_Attachment_Internal::_DATA.v1, &m_posoffset, Parent->GetWorldMatrix());
		Child->SetPosition(_Attachment_Internal::_DATA.v1);

		// Update the child world matrix immediately, IF it has any children of its own, so that 
		// its children will be starting with the correct transformation.  If it has no children
		// then we can afford to wait until the matrix is derived in the next cycle
		// TODO: Setting the pos/orient will prompt another recalculation of the child world matrix 
		// next frame, which is a little inefficient if we have also calculated it here.  Could zero-out the 
		// SpatialDataChanged flag when deriving the new world matrix, assuming this is the ONLY
		// reason for maintaining the flag
		if (Child->HasChildAttachments()) Child->RefreshPositionImmediate();
	}

	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// the object world matrix, position and orientation.  Less efficient than other methods since calculating
	// orientation requires decomposition of the world matrix and at least one sqrt
	void							Apply(void)
	{
		// Update the child object position based on the parent world matrix
		D3DXVec3TransformCoord(&_Attachment_Internal::_DATA.v1, &m_posoffset, Parent->GetWorldMatrix());
		Child->SetPosition(_Attachment_Internal::_DATA.v1);

		// Multiply the parent orientation by our offset quaternion to yield the child orientation
		// TODO: Does not currently account for the Ship::OrientationAdjustment since this was
		// causing non-affine transformation issues.  Resolve in future, or (ideally) find a way to 
		// remove the OrientationAdjustment field
		_Attachment_Internal::_DATA.q1 = (m_orientoffset * Parent->GetOrientation());
		D3DXQuaternionNormalize(&_Attachment_Internal::_DATA.q1, &_Attachment_Internal::_DATA.q1);
		Child->SetOrientation(_Attachment_Internal::_DATA.q1);
		//D3DXQuaternionRotationMatrix(&_Attachment_Internal::_DATA.q1, Parent->GetOrientationMatrix());
		//Child->SetOrientation(m_orientoffset * _Attachment_Internal::_DATA.q1);
		
		// Update the child world matrix immediately, IF it has any children of its own, so that 
		// its children will be starting with the correct transformation.  If it has no children
		// then we can afford to wait until the matrix is derived in the next cycle
		// TODO: Setting the pos/orient will prompt another recalculation of the child world matrix 
		// next frame, which is a little inefficient if we have also calculated it here.  Could zero-out the 
		// SpatialDataChanged flag when deriving the new world matrix, assuming this is the ONLY
		// reason for maintaining the flag
		if (Child->HasChildAttachments()) Child->RefreshPositionImmediate();
	}


	// Constructors to create new attachment objects
	Attachment(void);
	Attachment(T parent, T child);
	Attachment(T parent, T child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset);

	// Assign a new constraint between the two objects, making the attachment dynamic
	void							CreateConstraint(const D3DXVECTOR3 & axis, const D3DXVECTOR3 & parentpoint,
													 const D3DXVECTOR3 & childpoint, const D3DXQUATERNION & baseorient);

	// Removes any constraint in place between the two objects, making the attachment static again
	void							RemoveConstraint(void);

	// Rotate the parent object about the constraint, either by a delta rotation or by setting the rotation directly
	void							RotateParentAboutConstraint(float d_rad)			{ throw "Not implemented"; }
	void							SetParentRotationAboutConstraint(float rad)			{ throw "Not implemented"; }

	// Rotate the child object about the constraint, either by a delta rotation or by setting the rotation directly
	void							RotateChildAboutConstraint(float d_rad);
	void							SetChildRotationAboutConstraint(float rad);


	// Default destructor; no action to be taken, since there is no memory allocated on the heap for these objects
	~Attachment(void) { }

protected:

	// Position and orientation offsets
	D3DXVECTOR3						m_posoffset;
	D3DXQUATERNION					m_orientoffset;

	// Called whenever pos or orient offset changes, to precalculate the offset matrices used in rendering
	void							RecalculateOffsetParameters(void);

	// Offset matrix, precalcualted on any change of the offsets to save rendering time
	D3DXMATRIX						m_mat_offset;

};

// Sets position offset and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetPositionOffset(const D3DXVECTOR3 & off)
{
	m_posoffset = off;
	RecalculateOffsetParameters();
}

// Sets orientation offset and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetOrientationOffset(const D3DXQUATERNION & off)
{
	D3DXQuaternionNormalize(&m_orientoffset, &off);
	RecalculateOffsetParameters();
}

// Sets position and orientation offsets and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff)
{
	m_posoffset = poff;
	D3DXQuaternionNormalize(&m_orientoffset, &off);
	RecalculateOffsetParameters();
}

// Precalculates offset matrices in respose to a change in offset pos/orient, to save rendering time
template <typename T>
void Attachment<T>::RecalculateOffsetParameters(void)
{
	// Recalculate component matrices from the attachment offsets
	D3DXMatrixTranslation(&_Attachment_Internal::_DATA.m1, m_posoffset.x, m_posoffset.y, m_posoffset.z);
	D3DXMatrixRotationQuaternion(&_Attachment_Internal::_DATA.m2, &m_orientoffset);

	// Use them to calculate and store the combined offset matrix
	D3DXMatrixMultiply(&m_mat_offset, &_Attachment_Internal::_DATA.m2, &_Attachment_Internal::_DATA.m1);
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(void)
	: Parent(NULL), Child(NULL), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION), m_mat_offset(ID_MATRIX),
	Constraint(NULL)
{
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(T parent, T child)
	: Parent(parent), Child(child), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION), m_mat_offset(ID_MATRIX), 
	Constraint(NULL)
{
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(T parent, T child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset)
	: Parent(parent), Child(child), m_posoffset(posoffset), m_orientoffset(orientoffset), Constraint(NULL)
{
	RecalculateOffsetParameters();
}

// Assign a new constraint between the two objects, making the attachment dynamic
template <typename T>
void Attachment<T>::CreateConstraint(const D3DXVECTOR3 & axis, const D3DXVECTOR3 & parentpoint,
									 const D3DXVECTOR3 & childpoint, const D3DXQUATERNION & baseorient)
{
	// Remove any existing constraint before adding another
	RemoveConstraint();

	// Apply the effect of the attachment, and force an update of each object's world transform before 
	// determining constraint details.  This ensures the objects are in the right places when the constraint is formed
	Apply();
	Parent->RefreshPositionImmediate();
	Child->RefreshPositionImmediate();

	// Create a new constraint object to hold this data
	Constraint = new AttachmentConstraint();

	// Store initial state of the constraint
	Constraint->Axis = axis;					// Axis which the constraint lies in
	Constraint->ParentPoint = parentpoint;		// Point on the parent object which the axis passes through
	Constraint->ChildPoint = childpoint;		// Point on the child object which the axis passes through
	Constraint->BaseChildOrient = baseorient;	// Initial relative orientation from the parent to the child
	
	// Initial state represents zero rotation about the constraint axis
	Constraint->Rotation = 0.0f;

	// Cache the inverse of child base orientation (as parent child orientation)
	D3DXQuaternionInverse(&Constraint->BaseParentOrient, &baseorient);

	// Cache the inverse child translation matrix for runtime efficiency
	D3DXMatrixTranslation(&Constraint->InvChildPointOffset, -childpoint.x, -childpoint.y, -childpoint.z);
}

// Removes any constraint in place between the two objects, making the attachment static again
template <typename T>
void Attachment<T>::RemoveConstraint(void)
{
	// Delete the constraint if it exists
	if (Constraint) SafeDelete(Constraint);
}

// Rotate the child object about the constraint by a delta rotation
template <typename T>
void Attachment<T>::RotateChildAboutConstraint(float d_rad)
{
	// Make sure we have a valid constraint
	if (!Constraint) return;

	// Update the rotation value using this delta and call the main function
	SetChildRotationAboutConstraint(modf(Constraint->Rotation + d_rad, TWOPI));
}

// Rotate the child object about the constraint by setting the rotation directly
template <typename T>
void Attachment<T>::SetChildRotationAboutConstraint(float rad)
{
	/*
		q1 = dq, delta rotation quaternion
		q2 = neworient, relative orientation parent > child
		m1 = rotation matrix for neworient
		m2 = transform matrix for relative parent > child transform

	*/

	// Make sure we have a valid constraint
	if (!Constraint) return;

	// Store new rotation figure, and determine the corresponding rotation quaternion about our axis
	Constraint->Rotation = rad;
	D3DXQuaternionRotationAxis(&_Attachment_Internal::_DATA.q1, &Constraint->Axis, rad);					// q1 = dq (delta quaternion)

	// Determine the new overall relative orientation, accounting for base orientation
	_Attachment_Internal::_DATA.q2 = (Constraint->BaseChildOrient * _Attachment_Internal::_DATA.q1);		// q2 = neworient

	// Derive the matrix representing this new relative orientation, then build a transform matrix for the overall relative transform
	D3DXMatrixRotationQuaternion(&_Attachment_Internal::_DATA.m1, &_Attachment_Internal::_DATA.q2);			// m1 = rot matrix
	_Attachment_Internal::_DATA.m2 = (Constraint->InvChildPointOffset * _Attachment_Internal::_DATA.m1);	// m2 = transform

	// Eventual child position will be "ParentWorld( parent_point + transform_m2(child_point) )"
	// v1 = childpos, v2 = interimpos
	D3DXVec3TransformCoord(&_Attachment_Internal::_DATA.v1, &NULL_VECTOR, &_Attachment_Internal::_DATA.m2);			// v1 = childpos

	// Store these values in the attachment offset parameters
	m_posoffset = (Constraint->ParentPoint + _Attachment_Internal::_DATA.v1);
	m_orientoffset = _Attachment_Internal::_DATA.q2;
}


#endif





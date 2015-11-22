#pragma once

#ifndef __AttachmentH__
#define __AttachmentH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "FastMath.h"
#include "XML\\tinyxml.h"


// Struct representing a constraint between the two objects, for non-static attachments
// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct AttachmentConstraint : public ALIGN16<AttachmentConstraint>
{
	AXMVECTOR										Axis;					// Axis of rotation, in parent object space
	float											Rotation;				// Angle of rotation about the axis, in radians, from base orientation

	AXMVECTOR										ParentPoint;			// Point on the parent object which the axis passes through
	AXMVECTOR										ChildPoint;				// Point on the child object which the axis passes through
	AXMMATRIX										InvChildPointOffset;	// Cached translation matrix to (-ChildPoint)
	
	AXMVECTOR										BaseChildOrient;		// Base orientation of the child object, relative to the parent
	AXMVECTOR										BaseParentOrient;		// Base orientation of the parent object, relative to the 
																			// child.  Is Inverse(BaseChildOrient)

	// Default constructor to create new constraint data; no initialisation performed here
	AttachmentConstraint(void) { }
};

// Class requires 16-bit alignment, however cannot be specified for template class.  All member variables given force-alignment instead
template <typename T>
class Attachment : public ALIGN16<Attachment<T>>
{

public:
	T								Parent;					// Parent object
	T								Child;					// Child object

	AttachmentConstraint *			Constraint;				// Constraint between the two objects, for non-static attachments


	// Position offset of the child from the parent
	CMPINLINE XMVECTOR				GetPositionOffset(void) const						{ return m_posoffset; }
	void							SetPositionOffset(const FXMVECTOR off);

	// Orientation offset of the child from the parent
	CMPINLINE XMVECTOR				GetOrientationOffset(void) const					{ return m_orientoffset; }
	void							SetOrientationOffset(const FXMVECTOR off);

	// Set both offsets at once, for efficiency
	void							SetOffset(const FXMVECTOR poff, const FXMVECTOR qoff);

	// Update the parent or child object
	void							SetParent(T parent)									{ Parent = parent; }
	void							SetChild(T child)									{ Child = child; }

	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// the object world matrix, position and orientation.  Less efficient than other methods since calculating
	// orientation requires decomposition of the world matrix and at least one sqrt
	void							Apply(void)
	{
		// Update the child object position based on the parent world matrix
		Child->SetPosition(XMVector3TransformCoord(m_posoffset, Parent->GetWorldMatrix()));

		// Multiply the parent orientation by our offset quaternion to yield the child orientation
		// TODO: Does not currently account for the Ship::OrientationAdjustment since this was
		// causing non-affine transformation issues.  Resolve in future, or (ideally) find a way to 
		// remove the OrientationAdjustment field
		Child->SetOrientation(XMQuaternionMultiply(m_orientoffset, Parent->GetOrientation()));
		
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
	Attachment(T parent, T child, const FXMVECTOR posoffset, const FXMVECTOR orientoffset);

	// Assign a new constraint between the two objects, making the attachment dynamic
	void							CreateConstraint(const FXMVECTOR axis, const FXMVECTOR parentpoint,
													 const FXMVECTOR childpoint, const GXMVECTOR baseorient);

	// Removes any constraint in place between the two objects, making the attachment static again
	void							RemoveConstraint(void);

	// Rotate the parent object about the constraint, either by a delta rotation or by setting the rotation directly
	void							RotateParentAboutConstraint(float d_rad)			{ throw "Not implemented"; }
	void							SetParentRotationAboutConstraint(float rad)			{ throw "Not implemented"; }

	// Rotate the child object about the constraint, either by a delta rotation or by setting the rotation directly
	void							RotateChildAboutConstraint(float d_rad);
	void							SetChildRotationAboutConstraint(float rad);

	// Loads attachment data from XML specification
	Result							LoadAttachmentData(TiXmlElement *node);


	// Default destructor; no action to be taken, since there is no memory allocated on the heap for these objects
	~Attachment(void) { }

protected:

	// Position and orientation offsets
	AXMVECTOR						m_posoffset;
	AXMVECTOR						m_orientoffset;

	// Called whenever pos or orient offset changes, to precalculate the offset matrices used in rendering
	void							RecalculateOffsetParameters(void);

	// Offset matrix, precalcualted on any change of the offsets to save rendering time
	AXMMATRIX						m_mat_offset;

	// Load constraint parameters from XML, for use within an attachment
	Result							LoadAttachmentConstraintParameters(TiXmlElement *node, XMVECTOR & outAxis, XMVECTOR & outParentPoint, 
																	   XMVECTOR &outChildPoint, XMVECTOR & outBaseOrientation);
};

// Sets position offset and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetPositionOffset(const FXMVECTOR off)
{
	m_posoffset = off;
	RecalculateOffsetParameters();
}

// Sets orientation offset and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetOrientationOffset(const FXMVECTOR off)
{
	m_orientoffset = XMQuaternionNormalizeEst(off);
	RecalculateOffsetParameters();
}

// Sets position and orientation offsets and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetOffset(const FXMVECTOR poff, const FXMVECTOR qoff)
{
	m_posoffset = poff;
	m_orientoffset = XMQuaternionNormalizeEst(qoff);
	RecalculateOffsetParameters();
}

// Precalculates offset matrices in respose to a change in offset pos/orient, to save rendering time
template <typename T>
void Attachment<T>::RecalculateOffsetParameters(void)
{
	// Calculate and store the combined offset matrix
	m_mat_offset = XMMatrixMultiply(XMMatrixRotationQuaternion(m_orientoffset),
									XMMatrixTranslationFromVector(m_posoffset));
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
Attachment<T>::Attachment(T parent, T child, const FXMVECTOR posoffset, const FXMVECTOR orientoffset)
	: Parent(parent), Child(child), m_posoffset(posoffset), m_orientoffset(orientoffset), Constraint(NULL)
{
	RecalculateOffsetParameters();
}

// Assign a new constraint between the two objects, making the attachment dynamic
template <typename T>
void Attachment<T>::CreateConstraint(const FXMVECTOR axis, const FXMVECTOR parentpoint,
									 const FXMVECTOR childpoint, const GXMVECTOR baseorient)
{
	// Remove any existing constraint before adding another
	RemoveConstraint();

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
	Constraint->BaseParentOrient = XMQuaternionInverse(baseorient);

	// Cache the inverse child translation matrix for runtime efficiency
	Constraint->InvChildPointOffset = XMMatrixTranslationFromVector(XMVectorNegate(childpoint));

	// Set the constraint to its resting (0 rad) point, which will trigger a recaluation and set initial state for each object
	SetChildRotationAboutConstraint(0.0f);
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
	SetChildRotationAboutConstraint(std::fmodf(Constraint->Rotation + d_rad, TWOPI));
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

	// Store new rotation figure
	Constraint->Rotation = rad;
	
	// Determine the new overall relative orientation, by adding the delta quaternion to the base orientation
	XMVECTOR neworient = XMQuaternionMultiply(	Constraint->BaseChildOrient, 
												XMQuaternionRotationAxis(Constraint->Axis, rad));			// Neworient = (baseQ * deltaQ) q2

	// Derive the matrix representing this new relative orientation, then build a transform matrix for the overall relative transform
	XMMATRIX transform = XMMatrixMultiply(Constraint->InvChildPointOffset, XMMatrixRotationQuaternion(neworient)); // m2

	// Eventual child position will be "ParentWorld( parent_point + transform(child_point) )"
	// Store these new pos and orient values in the attachment offset parameters
	m_posoffset = XMVectorAdd(Constraint->ParentPoint, XMVector3TransformCoord(NULL_VECTOR, transform));
	m_orientoffset = neworient; 

	// Apply the attachment to show the change about this constraint
	// TOOD: Can use more efficient method rather than simply calling Apply() ?
	Apply();
}


template <class T>
Result Attachment<T>::LoadAttachmentData(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadAttachmentWithNullParameters;

	// Look at each child element in turn and pull data from them
	std::string key;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// Retrieve the xml node key and test it
		key = child->Value(); StrLowerC(key);

		// Test the hash against each expected field
		if (key == "posoffset")
		{
			SetPositionOffset(IO::GetVector3FromAttr(child));
		}
		else if (key == "orientoffset")
		{
			SetOrientationOffset(IO::GetVector4FromAttr(child));
		}
		else if (key == "constraint")
		{
			// Load all the constraint parameters from this sub-node
			XMVECTOR base_orient = ID_QUATERNION;
			XMVECTOR axis = UP_VECTOR, ppos = NULL_VECTOR, cpos = NULL_VECTOR;
			Result result = LoadAttachmentConstraintParameters(child, axis, ppos, cpos, base_orient);

			// If we could not load constraint data correctly then do not create the constraint
			if (result != ErrorCodes::NoError) continue;

			// Otherwise we can go ahead and create the constraint between these two objects
			CreateConstraint(axis, ppos, cpos, base_orient);
		}
	}

	// Return success
	return ErrorCodes::NoError;
}


// Load constraint parameters for use within an attachment
template <class T>
Result Attachment<T>::LoadAttachmentConstraintParameters(TiXmlElement *node, XMVECTOR & outAxis, XMVECTOR & outParentPoint,
														 XMVECTOR & outChildPoint, XMVECTOR & outBaseOrientation)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadAttachmentConstraintWithNullParams;

	// Look at each child element in turn and pull data from them
	std::string key;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// Retrieve the xml node key and test it
		key = child->Value(); StrLowerC(key);

		// Test the hash against each expected field
		if (key == "axis")
		{
			outAxis = IO::GetVector3FromAttr(child);
		}
		else if (key == "parentpoint")
		{
			outParentPoint = IO::GetVector3FromAttr(child);
		}
		else if (key == "childpoint")
		{
			outChildPoint = IO::GetVector3FromAttr(child);
		}
		else if (key == "baseorientation")
		{
			outBaseOrientation = IO::GetQuaternionFromAttr(child);
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

#endif





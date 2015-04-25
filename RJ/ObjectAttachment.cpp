#include "ObjectAttachment.h"
#include "iObject.h"
#include "FastMath.h"

// Initialise the static intermediate calculation structure
ObjectAttachment::_DATA_STRUCT ObjectAttachment::_DATA = ObjectAttachment::_DATA_STRUCT();

// Applies the effect of this attachment to the child object, recursively if necessary
void ObjectAttachment::ApplyAttachment(void)
{
	// Multiply the child offset matrix by it's parent's world matrix, yielding the child world matrix.  Repeat ad infinitum
	D3DXMatrixMultiply(&ObjectAttachment::_DATA.m1, &m_mat_offset, Parent->GetWorldMatrix());
	Child->SetWorldMatrix(ObjectAttachment::_DATA.m1);

	// Decompose to yield the world position and orientation of the child object
	D3DXMatrixDecompose(&ObjectAttachment::_DATA.v1, &ObjectAttachment::_DATA.q1, 
						&ObjectAttachment::_DATA.v2, &ObjectAttachment::_DATA.m1);
	Child->SetPosition(ObjectAttachment::_DATA.v2);
	Child->SetOrientation(&ObjectAttachment::_DATA.q1);
}

// Precalculates offset matrices in respose to a change in offset pos/orient, to save rendering time
void ObjectAttachment::RecalculateOffsetParameters(void)
{
	// Recalculate component matrices from the attachment offsets
	D3DXMatrixTranslation(&ObjectAttachment::_DATA.m1, m_posoffset.x, m_posoffset.y, m_posoffset.z);
	D3DXMatrixRotationQuaternion(&ObjectAttachment::_DATA.m2, &m_orientoffset);

	// Also calculate the combined offset matrix
	D3DXMatrixMultiply(&m_mat_offset, &ObjectAttachment::_DATA.m2, &ObjectAttachment::_DATA.m1);
}

// Sets position offset and recalculates parameters accordingly
void ObjectAttachment::SetPositionOffset(const D3DXVECTOR3 & off)
{
	m_posoffset = off;
	RecalculateOffsetParameters();
}

// Sets orientation offset and recalculates parameters accordingly
void ObjectAttachment::SetOrientationOffset(const D3DXQUATERNION & off)
{
	m_orientoffset = off;
	RecalculateOffsetParameters();
}

// Sets position and orientation offsets and recalculates parameters accordingly
void ObjectAttachment::SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff)
{
	m_posoffset = poff;
	m_orientoffset = qoff;
	RecalculateOffsetParameters();
}

ObjectAttachment::ObjectAttachment(void) 
	: Parent(NULL), Child(NULL), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION)
{
	RecalculateOffsetParameters();
}

ObjectAttachment::ObjectAttachment(iObject *parent, iObject *child)
	: Parent(parent), Child(child), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION)
{
	RecalculateOffsetParameters();
}

ObjectAttachment::ObjectAttachment(iObject *parent, iObject *child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset)
	: Parent(parent), Child(child), m_posoffset(posoffset), m_orientoffset(orientoffset)
{
	RecalculateOffsetParameters();
}

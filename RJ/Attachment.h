#pragma once

#ifndef __AttachmentH__
#define __AttachmentH__

#include "DX11_Core.h"
#include "CompilerSettings.h"

class _Attachment_Internal
{
public:
	// Struct holding temporary fields for intermediate calculations
	static struct _DATA_STRUCT { D3DXMATRIX m1, m2; D3DXVECTOR3 v1, v2; D3DXQUATERNION q1; } _DATA;
};


template <typename T>
class Attachment
{
public:
	T								Parent;					// Parent object
	T								Child;					// Child object

	// Position offset of the child from the parent
	CMPINLINE D3DXVECTOR3			GetPositionOffset(void) const						{ return m_posoffset; }
	void							SetPositionOffset(const D3DXVECTOR3 & off);

	// Orientation offset of the child from the parent
	CMPINLINE D3DXQUATERNION		GetOrientationOffset(void) const					{ return m_orientoffset; }
	void							SetOrientationOffset(const D3DXQUATERNION & off);

	// Set both offsets at once, for efficiency
	void							SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff);

	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// only the object world matrix and is relatively fast.  Use when position/orientation are not required
	void							Apply_TransformOnly(void);

	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// the object world matrix and stores its new position, and is relatively fast.  Use when orientation is not required
	void							Apply_TransformPositionOnly(void);

	// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
	// the object world matrix, position and orientation.  Less efficient than other methods since calculating
	// orientation requires decomposition of the world matrix and at least one sqrt
	void							Apply(void);

	// Constructors to create new attachment objects
	Attachment(void);
	Attachment(T parent, T child);
	Attachment(T parent, T child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset);

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
	m_orientoffset = off;
	RecalculateOffsetParameters();
}

// Sets position and orientation offsets and recalculates parameters accordingly
template <typename T>
void Attachment<T>::SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff)
{
	m_posoffset = poff;
	m_orientoffset = qoff;
	RecalculateOffsetParameters();
}

// Precalculates offset matrices in respose to a change in offset pos/orient, to save rendering time
template <typename T>
void Attachment<T>::RecalculateOffsetParameters(void)
{
	// Recalculate component matrices from the attachment offsets
	D3DXMatrixTranslation(&_Attachment_Internal::_DATA.m1, m_posoffset.x, m_posoffset.y, m_posoffset.z);
	D3DXMatrixRotationQuaternion(&_Attachment_Internal::_DATA.m2, &m_orientoffset);

	// Also calculate the combined offset matrix
	D3DXMatrixMultiply(&m_mat_offset, &_Attachment_Internal::_DATA.m2, &_Attachment_Internal::_DATA.m1);
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(void)
	: Parent(NULL), Child(NULL), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION), m_mat_offset(ID_MATRIX)
{
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(T parent, T child)
	: Parent(parent), Child(child), m_posoffset(NULL_VECTOR), m_orientoffset(ID_QUATERNION), m_mat_offset(ID_MATRIX)
{
}

// Creates a new attachment object
template <typename T>
Attachment<T>::Attachment(T parent, T child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset)
	: Parent(parent), Child(child), m_posoffset(posoffset), m_orientoffset(orientoffset)
{
	RecalculateOffsetParameters();
}

// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
// only the object world matrix and is relatively fast.  Use when position/orientation are not required
template <typename T>
void Attachment<T>::Apply_TransformOnly(void)
{
	// Multiply the child offset matrix by its parent's world matrix, yielding the child world matrix
	D3DXMatrixMultiply(&_Attachment_Internal::_DATA.m1, &m_mat_offset, Parent->GetWorldMatrix());
	Child->SetWorldMatrix(_Attachment_Internal::_DATA.m1);
}

// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
// the object world matrix and stores its new position, and is relatively fast.  Use when orientation is not required
template <typename T>
void Attachment<T>::Apply_TransformPositionOnly(void)
{
	// Multiply the child offset matrix by its parent's world matrix, yielding the child world matrix
	D3DXMatrixMultiply(&_Attachment_Internal::_DATA.m1, &m_mat_offset, Parent->GetWorldMatrix());
	Child->SetWorldMatrix(_Attachment_Internal::_DATA.m1);

	// Retrieve the new object position from its world matrix translation components
	Child->SetPosition(D3DXVECTOR3(_Attachment_Internal::_DATA.m1._41, _Attachment_Internal::_DATA.m1._42, _Attachment_Internal::_DATA.m1._43));
}

// Applies the effect of this attachment to the child object, recursively if necessary.  This method recalculates
// the object world matrix, position and orientation.  Less efficient than other methods since calculating
// orientation requires decomposition of the world matrix and at least one sqrt
template <typename T>
void Attachment<T>::Apply(void)
{
	// Multiply the child offset matrix by its parent's world matrix, yielding the child world matrix
	D3DXMatrixMultiply(&_Attachment_Internal::_DATA.m1, &m_mat_offset, Parent->GetWorldMatrix());
	Child->SetWorldMatrix(_Attachment_Internal::_DATA.m1);

	// Decompose to yield the world position and orientation of the child object
	D3DXMatrixDecompose(&_Attachment_Internal::_DATA.v1, &_Attachment_Internal::_DATA.q1,
		&_Attachment_Internal::_DATA.v2, &_Attachment_Internal::_DATA.m1);
	Child->SetPosition(_Attachment_Internal::_DATA.v2);
	Child->SetOrientation(&_Attachment_Internal::_DATA.q1);
}




#endif
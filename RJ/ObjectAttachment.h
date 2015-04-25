#pragma once

#ifndef __ObjectAttachmentH__
#define __ObjectAttachmentH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class iObject;


class ObjectAttachment
{
public:
	iObject*						Parent;					// Parent object
	iObject*						Child;					// Child object

	// Position offset of the child from the parent
	CMPINLINE D3DXVECTOR3			GetPositionOffset(void)								{ return m_posoffset; }
	void							SetPositionOffset(const D3DXVECTOR3 & off);

	// Orientation offset of the child from the parent
	CMPINLINE D3DXQUATERNION		GetOrientationOffset(void)							{ return m_orientoffset; }
	void							SetOrientationOffset(const D3DXQUATERNION & off);

	// Set both offsets at once, for efficiency
	void							SetOffset(const D3DXVECTOR3 & poff, const D3DXQUATERNION & qoff);

	// Applies the effect of this attachment to the child object, recursively if necessary
	void							ApplyAttachment(void);

	// Constructors to create new attachment objects
	ObjectAttachment(void);
	ObjectAttachment(iObject *parent, iObject *child);
	ObjectAttachment(iObject *parent, iObject *child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset);

	// Default destructor; no action to be taken, since there is no memory allocated on the heap for these objects
	~ObjectAttachment(void) { }

private:

	// Position and orientation offsets
	D3DXVECTOR3						m_posoffset;
	D3DXQUATERNION					m_orientoffset;

	// Called whenever pos or orient offset changes, to precalculate the offset matrices used in rendering
	void							RecalculateOffsetParameters(void);

	// Offset matrix, precalcualted on any change of the offsets to save rendering time
	D3DXMATRIX						m_mat_offset;

public:

	// Struct holding temporary fields for intermediate calculations
	static struct _DATA_STRUCT { D3DXMATRIX m1, m2; D3DXVECTOR3 v1, v2; D3DXQUATERNION q1; } _DATA;
};


#endif
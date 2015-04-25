#ifndef __SpaceObjectAttachmentH__
#define __SpaceObjectAttachmentH__

#pragma once

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "FastMath.h"
class iSpaceObject;


class SpaceObjectAttachment
{
public:
	iSpaceObject*					Parent;					// Parent object
	iSpaceObject*					Child;					// Child object

	// Position offset of the child from the parent
	CMPINLINE D3DXVECTOR3			GetPositionOffset(void)					{ return m_posoffset; }			
	void							SetPositionOffset(D3DXVECTOR3 off);	

	// Orientation offset of the child from the parent
	CMPINLINE D3DXQUATERNION		GetOrientationOffset(void)				{ return m_orientoffset; }		
	void							SetOrientationOffset(D3DXQUATERNION off);

	// Set both offsets at once, for efficiency
	void							SetOffset(D3DXVECTOR3 poff, D3DXQUATERNION qoff);

	// Applies the effect of this attachment to the child object, recursively if necessary
	void							ApplyAttachment(void);	

	// Locates an attachment in a vector of attachment objects
	static int						FindInList(vector<SpaceObjectAttachment*> *list, SpaceObjectAttachment *attach);

	// Static methods to create a new attachment
	static Result					Create(iSpaceObject *parent, iSpaceObject *child);
	static Result					Create(iSpaceObject *parent, iSpaceObject *child, const D3DXVECTOR3 poff, const D3DXQUATERNION qoff);

	// Static method to break an attachment, if it exists
	static Result					Detach(iSpaceObject *parent, iSpaceObject *child);
	Result							Detach(void);

	SpaceObjectAttachment(void);
	SpaceObjectAttachment(iSpaceObject *parent, iSpaceObject *child);
	SpaceObjectAttachment(iSpaceObject *parent, iSpaceObject *child, const D3DXVECTOR3 posoffset, const D3DXQUATERNION orientoffset);

	~SpaceObjectAttachment(void);

private:
	// Position and orientation offsets
	D3DXVECTOR3						m_posoffset;
	D3DXQUATERNION					m_orientoffset;

	// Called whenever pos or orient offset changes, to precalculate the offset matrices used in rendering
	void							RecalculateOffsetParameters(void);

	// Offset matrices, precalcualted on any change of the offset to save rendering time
	D3DXMATRIX						m_mat_posoffset;
	D3DXMATRIX						m_mat_orientoffset;
	D3DXMATRIX						m_mat_offset;
};


#endif
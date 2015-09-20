#pragma once
#ifndef __ArticulatedModelComponentH__
#define __ArticulatedModelComponentH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
class Model;

class ArticulatedModelComponent
{

public:

	// Default constructor
	ArticulatedModelComponent(void);

	// Reference to the model for this component
	const Model *						Model;

	// Retrieve or set the component position
	CMPINLINE D3DXVECTOR3				GetPosition(void) const							{ return m_position; }
	CMPINLINE void 						SetPosition(const D3DXVECTOR3 &pos)				{ m_position = pos; }

	// Retrieve or set the component orientation
	CMPINLINE D3DXQUATERNION			GetOrientation(void) const						{ return m_orientation; }
	CMPINLINE void 						SetOrientation(const D3DXQUATERNION &orient)	{ m_orientation = orient; }

	// Retrieve or set the component world matrix
	CMPINLINE D3DXMATRIX *				GetWorldMatrix(void)							{ return &m_worldmatrix; }
	CMPINLINE D3DXMATRIX 				GetWorldMatrixInstance(void) const				{ return m_worldmatrix; }
	CMPINLINE void						SetWorldMatrix(const D3DXMATRIX &m)				{ m_worldmatrix = m; }
	CMPINLINE void						SetWorldMatrix(D3DXMATRIX *m)					{ m_worldmatrix = *m; }

	// Performs an immediate recalculation of the world transform for this component
	void 								RefreshPositionImmediate(void);

	// Store a flag indicating whether this component has any child attachments; required for the Attachment<...> template.  However we
	// don't need to emulate the full attachment functionality within every component as we do for more complex objects; we can simply
	// set this flag whenever the component becomes the parent object in an attachment within the articulated model
	CMPINLINE bool						HasChildAttachments(void) const					{ return m_haschildren; }

	// Store a flag which indicates whether the component is already attached to a parent.  This flag is used during model initialisation 
	// to ensure that components are only ever attached to one parent
	CMPINLINE bool						HasParentAttachment(void) const					{ return m_hasparent; }

	// Notifies this component of whether it now has child objects attached
	CMPINLINE void						SetChildAttachmentState(bool has_children)		{ m_haschildren = has_children; }

	// Notifies this component of whether it is now attached to a parent object
	CMPINLINE void						SetParentAttachmentState(bool has_parent)		{ m_hasparent = has_parent; }

	// Default destructor
	~ArticulatedModelComponent(void);

	// Assigns the contents of another model component to this one
	void								operator=(const ArticulatedModelComponent & rhs);

	// Static working variables for inline intermediate calculations
	static struct						_calc_data_struct {
		D3DXVECTOR3						v1, v2, v3;
		D3DXQUATERNION					q1, q2, q3;
		D3DXMATRIX						m1, m2, m3;
	} _calc_data;


protected:

	// Position and orientation of the component in world space
	D3DXVECTOR3							m_position;
	D3DXQUATERNION						m_orientation;

	// World matrix for the component
	D3DXMATRIX							m_worldmatrix;

	// Flags indicating whether this object has a parent, or any child attachments
	bool								m_hasparent;
	bool								m_haschildren;

};


#endif
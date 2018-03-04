#pragma once
#ifndef __ArticulatedModelComponentH__
#define __ArticulatedModelComponentH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "ModelInstance.h"
class Model;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ArticulatedModelComponent : public ALIGN16<ArticulatedModelComponent>
{

public:

	// Default constructor
	ArticulatedModelComponent(void);

	// Reference to the model for this component
	ModelInstance						Model;

	// Retrieve or set the component position
	CMPINLINE XMVECTOR					GetPosition(void) const							{ return m_position; }
	CMPINLINE void 						SetPosition(const FXMVECTOR pos)				{ m_position = pos; }

	// Retrieve or set the component orientation
	CMPINLINE XMVECTOR					GetOrientation(void) const						{ return m_orientation; }
	CMPINLINE void 						SetOrientation(const FXMVECTOR orient)			{ m_orientation = orient; }

	// Set the component size
	CMPINLINE void						SetSize(const FXMVECTOR size)					{ Model.SetSize(size); }
	CMPINLINE void						SetSize(const XMFLOAT3 & size)					{ SetSize(XMLoadFloat3(&size)); }
	CMPINLINE void						SetSize(float max_dimension)					{ Model.SetSize(max_dimension); }

	// Retrieve or set the component world matrix
	CMPINLINE XMMATRIX					GetWorldMatrix(void) const						{ return m_worldmatrix; }
	CMPINLINE void RJ_XM_CALLCONV		SetWorldMatrix(const FXMMATRIX m)				{ m_worldmatrix = m; }

	// Set all spatial components at once, to reduce method calls when all information is known
	CMPINLINE void						SetAllSpatialData(	const FXMVECTOR position, const FXMVECTOR orientation,
															const CXMMATRIX worldmatrix)
	{
		m_position = position;
		m_orientation = orientation;
		SetWorldMatrix(XMMatrixMultiply(Model.GetWorldMatrix(), worldmatrix));	// Account for inherent model transform first
	}

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


protected:

	// Position and orientation of the component in world space
	AXMVECTOR							m_position;
	AXMVECTOR							m_orientation;

	// World matrix for the component
	AXMMATRIX							m_worldmatrix;

	// Flags indicating whether this object has a parent, or any child attachments
	bool								m_hasparent;
	bool								m_haschildren;

};


#endif
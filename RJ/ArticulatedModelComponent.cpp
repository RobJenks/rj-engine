#include "FastMath.h"

#include "ArticulatedModelComponent.h"


// Default constructor
ArticulatedModelComponent::ArticulatedModelComponent(void)
	: 
	Model(NULL), m_position(NULL_VECTOR), m_orientation(ID_QUATERNION), m_worldmatrix(ID_MATRIX), m_lastworld(ID_MATRIX), 
	m_hasparent(false), m_haschildren(false)
{
}

// Performs an immediate recalculation of the world transform for this component
void ArticulatedModelComponent::RefreshPositionImmediate(void)
{
	// Record the last world transform, if it was in fact calculated in the prior frame
	if (m_worldcurrent.WasSetInPriorFrame())
	{
		m_lastworld = m_worldmatrix;
	}

	// Set world matrix: World = Rotation * Translation
	SetWorldMatrix(XMMatrixMultiply(XMMatrixMultiply(
		Model.GetWorldMatrix(),
		XMMatrixRotationQuaternion(m_orientation)),
		XMMatrixTranslationFromVector(m_position)));

	// Record the fact that we have just recalculated our world transform
	m_worldcurrent.Set();
}

// Assigns the contents of another model component to this one.  Resets the hasparent/haschild flags,
// since these are driven by the attachments between components in the new model
void ArticulatedModelComponent::operator=(const ArticulatedModelComponent & rhs)
{
	Model = rhs.Model;
	SetPosition(rhs.GetPosition());
	SetOrientation(rhs.GetOrientation());
	SetWorldMatrix(rhs.GetWorldMatrix());
	SetParentAttachmentState(false);
	SetChildAttachmentState(false);

	m_lastworld = rhs.m_lastworld;
	m_worldcurrent = rhs.m_worldcurrent;
}

// Default destructor
ArticulatedModelComponent::~ArticulatedModelComponent(void)
{

}
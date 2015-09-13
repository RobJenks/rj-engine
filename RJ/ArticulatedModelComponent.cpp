#include "FastMath.h"

#include "ArticulatedModelComponent.h"


// Static intermediate data struct to support high-performance calculations
ArticulatedModelComponent::_calc_data_struct ArticulatedModelComponent::_calc_data = ArticulatedModelComponent::_calc_data_struct();


// Default constructor
ArticulatedModelComponent::ArticulatedModelComponent(void)
	: 
	Model(NULL), m_position(NULL_VECTOR), m_orientation(ID_QUATERNION), m_worldmatrix(ID_MATRIX), 
	m_hasparent(false), m_haschildren(false)
{
}


// Performs an immediate recalculation of the world transform for this component
void ArticulatedModelComponent::RefreshPositionImmediate(void)
{
	D3DXMatrixRotationQuaternion(&ArticulatedModelComponent::_calc_data.m1, &m_orientation);
	D3DXMatrixTranslation(&_calc_data.m2, m_position.x, m_position.y, m_position.z);
	SetWorldMatrix(_calc_data.m1 * _calc_data.m2);
}


// Default destructor
ArticulatedModelComponent::~ArticulatedModelComponent(void)
{

}
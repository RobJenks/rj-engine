#include "ModelInstance.h"
#include "FastMath.h"
#include "Model.h"
using namespace DirectX;

ModelInstance::ModelInstance(void)
	:
	m_model(NULL), 
	m_world(ID_MATRIX)
{
}

ModelInstance::ModelInstance(Model *model)
	:
	m_model(model), 
	m_world(ID_MATRIX)
{
}

ModelInstance::ModelInstance(const std::string & model)
	:
	m_model(Model::GetModel(model)), 
	m_world(ID_MATRIX)
{
}

void ModelInstance::SetScaleFactor(const FXMVECTOR scale)
{
	m_world = XMMatrixScalingFromVector(scale);
}

void ModelInstance::SetSize(const FXMVECTOR size)
{
	if (!m_model || !m_model->Geometry) return;
	
	XMVECTOR base_size = XMLoadFloat3( &(m_model->Geometry.get()->ModelSize) );
	XMVECTOR component_scale = XMVectorDivide(size, base_size);

	// Uniform-scale by the smallest absolute component scale factor, so that the final size 
	// will be <= target size depending on relative proportions.  After this operation
	// the result.x value will represent min { x, y, z }
	XMVECTOR comp_scale_abs = XMVectorAbs(component_scale);
	XMVECTOR min_comp = XMVectorMin(XMVectorMin(comp_scale_abs,
		XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_W>(comp_scale_abs)),
		XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W>(comp_scale_abs));
	
	// Uniform scale based on the x component only
	SetScaleFactor(XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X>(min_comp));
}

void ModelInstance::SetExactSize(const FXMVECTOR size)
{
	if (!m_model || !m_model->Geometry) return;

	XMVECTOR base_size = XMLoadFloat3(&(m_model->Geometry.get()->ModelSize));
	XMVECTOR component_scale = XMVectorDivide(size, base_size);

	SetScaleFactor(component_scale);
}

// Set the model definition.  Will reset any size/scaling factor that has already been applied
CMPINLINE void ModelInstance::SetModel(Model *model)
{
	m_model = model;
	m_world = ID_MATRIX;
}

// Derive properties of this model instance, based upon the underlying model definition and our per-instance parameters
XMVECTOR ModelInstance::DetermineModelInstanceSize(void) const
{
	if (!m_model) return NULL_VECTOR;
	ModelData *data = m_model->Geometry.get();
	if (!data) return NULL_VECTOR;

	return XMVector3TransformCoord(XMLoadFloat3(&data->ModelSize), m_world);
}

// Derive properties of this model instance, based upon the underlying model definition and our per-instance parameters
XMVECTOR ModelInstance::DetermineModelInstanceMinBounds(void) const
{
	if (!m_model) return NULL_VECTOR;
	ModelData *data = m_model->Geometry.get();
	if (!data) return NULL_VECTOR;

	return XMVector3TransformCoord(XMLoadFloat3(&data->MinBounds), m_world);
}

// Derive properties of this model instance, based upon the underlying model definition and our per-instance parameters
XMVECTOR ModelInstance::DetermineModelInstanceMaxBounds(void) const
{
	if (!m_model) return NULL_VECTOR;
	ModelData *data = m_model->Geometry.get();
	if (!data) return NULL_VECTOR;

	return XMVector3TransformCoord(XMLoadFloat3(&data->MaxBounds), m_world);
}



ModelInstance::~ModelInstance(void)
{
}

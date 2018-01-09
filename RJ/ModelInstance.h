#pragma once

#include <string>
#include "CompilerSettings.h"
#include "DX11_Core.h"
class Model;


class ModelInstance
{
public:

	ModelInstance(void);
	ModelInstance(Model *model);
	ModelInstance(const std::string & model);

	CMPINLINE Model *					GetModel(void) { return m_model; }
	CMPINLINE const XMMATRIX &			GetWorldMatrix(void) const { return m_world; }

	// Set the model definition.  Will reset any size/scaling factor that has already been applied
	CMPINLINE void						SetModel(Model *model);

	// Scales geometry by a multiple of its base size
	void								SetScaleFactor(const FXMVECTOR scale);
	CMPINLINE void						SetScaleFactor(const XMFLOAT3 & scale) { return SetScaleFactor(XMLoadFloat3(&scale)); }
	CMPINLINE void						SetScaleFactor(float uniform_scale) { return SetScaleFactor(XMVectorReplicate(uniform_scale)); }
	
	// Sets the size of the model, preserving relative proportions.  Model will be <= target size depending on relative proportions
	void								SetSize(const FXMVECTOR size);
	CMPINLINE void						SetSize(const XMFLOAT3 & size) { return SetSize(XMLoadFloat3(&size)); }
	CMPINLINE void						SetSize(float uniform_size) { return SetSize(XMVectorReplicate(uniform_size)); }

	// Sets the size of the model.  Relative proportions will NOT be preserved and the model size will be == target size after the transformation
	void								SetExactSize(const FXMVECTOR size);
	CMPINLINE void						SetExactSize(const XMFLOAT3 & size) { return SetExactSize(XMLoadFloat3(&size)); }
	CMPINLINE void						SetExactSize(float uniform_size) { return SetExactSize(XMVectorReplicate(uniform_size)); }

	// Derive properties of this model instance, based upon the underlying model definition and our per-instance parameters
	XMVECTOR							DetermineModelInstanceSize(void) const;
	XMVECTOR							DetermineModelInstanceMinBounds(void) const;
	XMVECTOR							DetermineModelInstanceMaxBounds(void) const;

	~ModelInstance(void);

private:

	Model *								m_model;		// Pointer to the central model definition
	XMMATRIX							m_world;		// Base world matrix, based upon e.g. desired model instance size

};
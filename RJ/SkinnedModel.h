#pragma once

#ifndef __SkinnedModelH__
#define __SkinnedModelH__


#include <vector>
#include "DX11_Core.h"
#include <windows.h>
#include "CompilerSettings.h"
#include "Utility.h"
#include "SkinnedData.h"
#include "MeshGeometry.h"
class SkinnedModelInstance;

// This class does not have any special alignment requirements
class SkinnedModel
{
public:
	struct Vertex_PosNormalTexTanSkinned
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT4 TangentU;
		XMFLOAT3 Weights;
		BYTE BoneIndices[4];
	};
	
	struct SM_Material
	{
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = SpecPower
		XMFLOAT4 Reflect;

		SM_Material() { ZeroMemory(this, sizeof(this)); }
		SM_Material(XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular, XMFLOAT4 reflect)
					{ Ambient = ambient; Diffuse = diffuse; Specular = specular; Reflect = reflect; }
	};

	SkinnedModel(ID3D11Device* device, const std::string & code, const std::string & modelFilename, const std::string & texturePath);
	~SkinnedModel();

	UINT SubsetCount;

	std::vector<SM_Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex_PosNormalTexTanSkinned> Vertices;
	std::vector<USHORT> Indices;
	std::vector<MeshGeometry::Subset> Subsets;

	MeshGeometry ModelMesh;
	SkinnedData SkinnedData;

	// Store the model bounds in each dimension
	XMFLOAT3 Bounds[2];				// [0] = min bounds, [1] = max bounds

	// Methods to generate new instances of this model
	void CreateInstance(SkinnedModelInstance &instance);
	void CreateInstance(SkinnedModelInstance *instance);

	// Passthrough method to look up and return a reference to an animation by its string code
	CMPINLINE const AnimationClip * GetAnimation(const std::string & code)	{ return SkinnedData.GetAnimation(code); }

	// Inline methods to retrieve and set the model code
	CMPINLINE std::string				GetCode(void)					{ return m_code; }
	CMPINLINE void						SetCode(std::string & code)		{ m_code = code; }

	// Methods to get and set the default animation for this model
	CMPINLINE const AnimationClip *		GetDefaultAnimation(void) const	{ return m_defaultanimation; }
	void								SetDefaultAnimation(const std::string & anim);

	// Retrieve the base model mesh size.  Read-only, derived upon loading the model data
	CMPINLINE XMFLOAT3					GetBaseMeshSize(void) const		{ return m_meshsize; }

	// Retrieve the actual model size (equivalent to BaseMeshSize * ScaleFactor)
	CMPINLINE XMFLOAT3					GetModelSize(void) const		{ return m_modelsize; }

	// Sets the size of this mode.  Can either accept a full Vector3, to scale in all dimensions, or just one element
	// of a Vector3 in which case the model will be scaled to maintain proportions in the other two dimensions
	void								SetModelSize(const XMFLOAT3 & size);

	// Returns the scaling factor applied to the base model mesh, in order to give the desired ultimate model size.  Publicly read-only
	CMPINLINE XMFLOAT3					GetScaleFactor(void) const		{ return m_scalefactor; }

	// Returns the scale/rotation/translation adjustment to be applied for this model at render time
	CMPINLINE XMFLOAT4X4				GetScaleRotationTranslationAdjustment(void) const		{ return m_scalerottransadj; }
	CMPINLINE XMFLOAT4X4 * 				GetScaleRotationTranslationAdjustmentReference(void)	{ return &m_scalerottransadj; }
	CMPINLINE float						GetTranslationFixAdjustment(void) const					{ return m_transadj; }

	// Sets the view offset as a percentage of model size.  Measured from the x/z centre between the model feet
	void								SetViewOffsetPercentage(const XMFLOAT3 & offset_pc);

	// Retrieve the view offset as an absolute translation, or as a percentage of total model size
	CMPINLINE XMFLOAT3					GetViewOffset(void) const				{ return m_absoluteviewoffset; }
	CMPINLINE XMFLOAT3					GetViewOffsetPercentage(void) const		{ return m_viewoffset; }

protected:

	std::string							m_code;
	const AnimationClip *				m_defaultanimation;

	// Store the base mesh size upon loading the mesh
	XMFLOAT3							m_meshsize;

	// We will store the ultimate model size in two ways; as an absolute size vector, and as 
	// the scaling factor to be applied to the base mesh
	XMFLOAT3							m_modelsize;
	XMFLOAT3							m_scalefactor;

	// Precalculated render matrices and other variables that can be initialised once for efficiency
	XMFLOAT4X4							m_scalerottransadj;
	float								m_transadj;

	// View offset as a percentage of model size.  Calculated as an offset from the x/z centre point, between the actor's feet
	// So (0.0f, 0.9f, 0.0f) would be a reasonable approximation for a human model's head.  Absolute offset is read-only and derived
	XMFLOAT3							m_viewoffset;				// Percentage of model size
	XMFLOAT3							m_absoluteviewoffset;		// Absolute offset (read-only, derived)

	// Methods to set the mesh scaling factors and recalculated derived fields.  Protected; only to be called internally.  
	// External methods should call SetModelSize() with desired ultimate model dimensions
	void								SetScaleFactor(XMFLOAT3 sf);
	
};

class SkinnedModelInstance
{
public:
	SkinnedModel * Model;
	float TimePos;
	const AnimationClip * CurrentAnimation;
	XMFLOAT4X4 World;
	std::vector<XMFLOAT4X4> FinalTransforms;

	void Update(float dt);

	SkinnedModelInstance(void)
	{
		Model = NULL;
		TimePos = 0.0f;
		CurrentAnimation = NULL;
		XMStoreFloat4x4(&World, XMMatrixIdentity());
	}
};


#endif
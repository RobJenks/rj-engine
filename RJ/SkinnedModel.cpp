#include "FastMath.h"
#include "Utility.h"
#include "SkinnedModel.h"
#include "GameDataExtern.h"
#include "LoadM3d.h"
#include "Texture.h"

SkinnedModel::SkinnedModel(ID3D11Device* device, const std::string &code, const std::string& modelFilename, const std::string& texturePath)
{
	m_code = code;

	std::vector<M3dMaterial> mats;
	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(BuildStrFilename(D::DATA, modelFilename), Vertices, Indices, Subsets, mats, SkinnedData, Bounds);

	ModelMesh.SetVertices(device, &Vertices[0], Vertices.size());
	ModelMesh.SetIndices(device, &Indices[0], Indices.size());
	ModelMesh.SetSubsetTable(Subsets);

	SubsetCount = mats.size();

	for(UINT i = 0; i < SubsetCount; ++i)
	{
		Mat.push_back(mats[i].Mat);

		ID3D11ShaderResourceView* diffuseMapSRV = Texture::CreateSRV(concat(D::DATA)("\\")(texturePath)("\\")(mats[i].DiffuseMapName).str());
		DiffuseMapSRV.push_back(diffuseMapSRV);		// TODO: Check for NULL in case of failure?  Or will render without texture if == NULL?

		ID3D11ShaderResourceView* normalMapSRV = Texture::CreateSRV(concat(D::DATA)("\\")(texturePath)("\\")(mats[i].NormalMapName).str());
		NormalMapSRV.push_back(normalMapSRV);		// TODO: Check for NULL in case of failure?  Or will render without texture if == NULL?
	}

	// Set default values
	m_defaultanimation = NULL;
	XMStoreFloat4x4(&m_scalerottransadj, XMMatrixIdentity());
	m_transadj = 0.0f;
	m_viewoffset = m_absoluteviewoffset = D3DXVECTOR3(0.0f, 0.9f, 0.0f);

	// Store the base size of the model mesh
	m_meshsize = D3DXVECTOR3(	max((Bounds[1].x - Bounds[0].x), Game::C_EPSILON),
								max((Bounds[1].y - Bounds[0].y), Game::C_EPSILON),
								max((Bounds[1].z - Bounds[0].z), Game::C_EPSILON));

	// Set a default model size equal to the base mesh size
	SetModelSize(m_meshsize);
}

// Method to generate new instances of this model
void SkinnedModel::CreateInstance(SkinnedModelInstance &instance)
{
	instance.Model = this;
	instance.TimePos = 0.0f;
	instance.CurrentAnimation = this->GetDefaultAnimation();			// TODO: change?
	instance.FinalTransforms.resize(this->SkinnedData.BoneCount());
	XMStoreFloat4x4(&(instance.World), XMMatrixIdentity());				// Added just to make sure this is initialised to ID for now
}
// Method to generate new instances of this model
void SkinnedModel::CreateInstance(SkinnedModelInstance *instance)
{
	if (!instance) return;

	instance->Model = this;
	instance->TimePos = 0.0f;
	instance->CurrentAnimation = this->GetDefaultAnimation();			// TODO: change?
	instance->FinalTransforms.resize(this->SkinnedData.BoneCount());
	XMStoreFloat4x4(&(instance->World), XMMatrixIdentity());			// Added just to make sure this is initialised to ID for now
}

// Set the default animation for this model
void SkinnedModel::SetDefaultAnimation(const std::string & anim)
{
	// Attempt to get a reference to this animation and store it (even if NULL, in case we didn't find it)
	m_defaultanimation = SkinnedData.GetAnimation(anim);
}

// Sets the size of this mode.  Can either accept a full Vector3, to scale in all dimensions, or just one element
// of a Vector3 in which case the model will be scaled to maintain proportions in the other two dimensions
void SkinnedModel::SetModelSize(D3DXVECTOR3 size)
{
	// First, test whether we have a full size vector, or whether one component has been provided for derivation of the other two
	if (size.x > Game::C_EPSILON && size.y > Game::C_EPSILON && size.z > Game::C_EPSILON)
	{
		// We have been given a full size vector; store the new size
		m_modelsize = size;

		// Calculate the scaling factor that will result in this ultimate model size and apply it.  We guarantee that MeshSize 
		// has all non-zero components upon loading the mesh, so no need to validate here as well
		SetScaleFactor(XMFLOAT3(size.x / m_meshsize.x, size.y / m_meshsize.y, size.z / m_meshsize.z));
	}
	else
	{
		// We do not have a full size vector; test if we have a vector correctly containing one component.  Again, we have already
		// guaranteed that the MeshSize is non-zero in all components so do not need to check it again here
		float scale;
		if		(size.x > Game::C_EPSILON && size.y <= Game::C_EPSILON && size.z <= Game::C_EPSILON) scale = (size.x / m_meshsize.x);
		else if (size.x <= Game::C_EPSILON && size.y > Game::C_EPSILON && size.z <= Game::C_EPSILON) scale = (size.y / m_meshsize.y);
		else if (size.x <= Game::C_EPSILON && size.y <= Game::C_EPSILON && size.z > Game::C_EPSILON) scale = (size.z / m_meshsize.z);
		else																						 scale = 1.0f;

		// Set the ultimate model size based on this scaling factor; default will be 1.0 if invalid values were supplied
		m_modelsize = D3DXVECTOR3(m_meshsize.x * scale, m_meshsize.y * scale, m_meshsize.z * scale);

		// Set the scaling factor that we have already calculated
		SetScaleFactor(XMFLOAT3(scale, scale, scale));
	}
}

// Changes the scale factor applied to this model
void SkinnedModel::SetScaleFactor(XMFLOAT3 sf)
{
	// Store the new scaling factor
	m_scalefactor = sf;

	// Scaling to 0.0 in any dimension is disallowed
	if (m_scalefactor.x < Game::C_EPSILON) m_scalefactor.x = 1.0f;
	if (m_scalefactor.y < Game::C_EPSILON) m_scalefactor.y = 1.0f;
	if (m_scalefactor.z < Game::C_EPSILON) m_scalefactor.z = 1.0f;

	// Safety check to prevent scaling over the maximum model size limit
	if ((m_meshsize.x * m_scalefactor.x) > Game::C_MODEL_SIZE_LIMIT) m_scalefactor.x = (Game::C_MODEL_SIZE_LIMIT / max(m_meshsize.x, Game::C_EPSILON));
	if ((m_meshsize.y * m_scalefactor.y) > Game::C_MODEL_SIZE_LIMIT) m_scalefactor.y = (Game::C_MODEL_SIZE_LIMIT / max(m_meshsize.y, Game::C_EPSILON));
	if ((m_meshsize.z * m_scalefactor.z) > Game::C_MODEL_SIZE_LIMIT) m_scalefactor.z = (Game::C_MODEL_SIZE_LIMIT / max(m_meshsize.z, Game::C_EPSILON));

	// Recalculate the translation adjustment for use at render time (to ensure min(modelvertices.y) == 0)) and all vertices are >=0
	// "-(Bounds[0].y * m_scalefactor.y)" will translate model feet to y=0; " - (m_modelsize.y * 0.5f)" translates model centre to y=0
	m_transadj = -(Bounds[0].y * m_scalefactor.y) - (m_modelsize.y * 0.5f);

	// Recalculate the scale/rotate/translation adjustment for use at render time
	XMMATRIX xadj =   XMMatrixScaling(m_scalefactor.x, m_scalefactor.y, m_scalefactor.z)	// Scale to desired ultimate model size
					* XMMatrixRotationY(PI)													// Rotation fix to ensure facing forwards
					* XMMatrixTranslation(0.0f, m_transadj, 0.0f);							// Translate to ensure model 'feet' are at ground level
	XMStoreFloat4x4(&m_scalerottransadj, xadj);

	// Recalculate the model view offset, which will be affected by any change in model size
	SetViewOffsetPercentage(m_viewoffset);
}

// Sets the view offset as a percentage of model size.  Measured from the x/z centre between the model feet
// Valid values within the model bounds (though this is not necessary) are therefore ([-0.5 +0.5], [0.0 1.0], [-0.5 +0.5])
void SkinnedModel::SetViewOffsetPercentage(const D3DXVECTOR3 & offset_pc)
{
	// Store the percentage offsets that were provided
	m_viewoffset = offset_pc;

	// Recalculate the absolute view offset based on this, and the actual model size
	m_absoluteviewoffset = D3DXVECTOR3(	(m_viewoffset.x * m_modelsize.x),
										(m_viewoffset.y * m_modelsize.y) - 0.5f,		// Subtract 0.5 since Y is measured from ground level, for convenience
										(m_viewoffset.z * m_modelsize.z));
}

// Default destructor
SkinnedModel::~SkinnedModel()
{
}

void SkinnedModelInstance::Update(float dt)
{
	if (!CurrentAnimation) return;

	TimePos += dt;
	Model->SkinnedData.GetFinalTransforms(CurrentAnimation, TimePos, FinalTransforms);

	// Loop animation
	if(TimePos > CurrentAnimation->GetClipEndTime()) /* Model->SkinnedData.GetClipEndTime(ClipName)) */
		TimePos = 0.0f;
}
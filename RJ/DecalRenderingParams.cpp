#include "DecalRenderingParams.h"
#include "FastMath.h"

// Constructor
DecalRenderingParams::DecalRenderingParams(void)
	:
	DecalRenderingParams(NULL, ONE_FLOAT4, NULL_FLOAT4)
{
}

// Constructor
DecalRenderingParams::DecalRenderingParams(const TextureDX11 * texture, const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour)
	:
	m_rendermode(DecalRenderingMode::ScreenSpace), 
	m_texture(texture),
	m_basecolour(basecolour),
	m_outlinecolour(outlinecolour),
	m_outlinewidthfactor(0.0f),
	m_smoothingfactor(1.0f / 4.0f)
{
}

// Add a new instance for rendering
void DecalRenderingParams::AddInstance(const FXMMATRIX world, const XMFLOAT4 & uv_shift_scale)
{
	m_instances.push_back(RM_Instance(world, RM_Instance::SORT_KEY_RENDER_LAST, uv_shift_scale, InstanceFlags::DEFAULT_INSTANCE_FLAGS));
}


// Clones data from another group into this one.  Instance data is NOT copied
void DecalRenderingParams::CloneData(const DecalRenderingParams & other)
{
	SetRenderMode(other.GetRenderingMode());
	SetTexture(other.GetTexture());
	SetBaseColour(other.GetBaseColour());
	SetOutlineColour(other.GetOutlineColour());
	SetOutlineWidthFactor(other.GetOutlineWidthFactor());
	SetSmoothingFactor(other.GetSmoothingFactor());
}



// Destructor
DecalRenderingParams::~DecalRenderingParams(void)
{
}
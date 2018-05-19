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
	m_texture(texture), 
	m_basecolour(basecolour), 
	m_outlinecolour(outlinecolour)
{
}

// Add a new instance for rendering
void DecalRenderingParams::AddInstance(const FXMMATRIX world, const XMFLOAT4 & uv_shift_scale)
{
	m_instances.push_back(RM_Instance(world, RM_Instance::SORT_KEY_RENDER_LAST, uv_shift_scale));
}






// Destructor
DecalRenderingParams::~DecalRenderingParams(void)
{
}
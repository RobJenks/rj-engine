#include "DecalRenderer.h"
#include "CoreEngine.h"

// Constructor
DecalRenderer::DecalRenderer(void)
{
}

// Prepare the decal renderer for a new frame
void DecalRenderer::BeginFrame(void)
{
	// Clear all render group data ready for the current frame
	ClearAllRenderGroups();

	// Start a new render group so that we are guaranteed to have an 'active' group at all times
	StartNewRenderGroup();
}

// Set properties that are applied to all rendered decals
void DecalRenderer::SetTexture(const TextureDX11 * texture)
{
	// Start a new decal rendering group if this changes the rendering params, and if we do already have one active
	StartNewRenderGroupIf( texture != GetActiveRenderGroup().GetTexture() );

	// Store this new parameter in the active group
	GetActiveRenderGroup().SetTexture(texture);
}

// Set properties that are applied to all rendered decals
void DecalRenderer::SetBaseColour(const XMFLOAT4 & colour)
{
	// Start a new decal rendering group if this changes the rendering params, and if we do already have one active
	StartNewRenderGroupIf( Float4NotEqual(colour, GetActiveRenderGroup().GetBaseColour()) );

	// Store this new parameter in the active group
	GetActiveRenderGroup().SetBaseColour(colour);
}

// Set properties that are applied to all rendered decals
void DecalRenderer::SetOutlineColour(const XMFLOAT4 & outline)
{
	// Start a new decal rendering group if this changes the rendering params, and if we do already have one active
	StartNewRenderGroupIf(Float4NotEqual(outline, GetActiveRenderGroup().GetOutlineColour()));

	// Store this new parameter in the active group
	GetActiveRenderGroup().SetOutlineColour(outline);
}

// Get the currently-active rendering group
DecalRenderingParams & DecalRenderer::GetActiveRenderGroup(void)
{
	return (m_rendergroups.back());
}

// Starts a new decal rendering group
void DecalRenderer::StartNewRenderGroup(void)
{
	m_rendergroups.push_back(DecalRenderingParams());
}

// Start a new render group if the current group is in use, and the provided condition resolves to true
void DecalRenderer::StartNewRenderGroupIf(bool condition)
{
	if (condition && GetActiveRenderGroup().IsInUse())
	{
		StartNewRenderGroup();
	}
}

// Clear all render groups and associated data
void DecalRenderer::ClearAllRenderGroups(void)
{
	m_rendergroups.clear();
}




// Render a decal to the given screen-space location
void DecalRenderer::RenderDecalScreen(const FXMVECTOR location, const FXMVECTOR size)
{
	RenderDecalScreenInstance(location, size, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	// No shift, unit scale
}

// Render the specified subset of a decal to the given screen-space location
void DecalRenderer::RenderDecalScreen(const FXMVECTOR location, XMFLOAT2 size, UINTVECTOR2 tex_min, UINTVECTOR2 tex_max)
{
	// There must be an active texture assigned in order to make this call
	const TextureDX11 * texture = GetActiveRenderGroup().GetTexture();
	if (!texture) return;

	// Size of decal = (6, 4)
	// Desired texmin = (1, 3), texmax = (3, 5) -> texsize = (2, 2)
	// -> uv_shift = (1, 3)
	// -> uv_scale = (2, 2)/(6, 4) = (0.333, 0.5)
	// ---> in shader, vertex.uv = (uv_shift + (vertex.uv * uv_scale))
	XMFLOAT2 texsize = (tex_max - tex_min).ToFloat();
	XMFLOAT2 uv_scale = Float2Divide(texsize, size);

	RenderDecalScreenInstance(location, XMLoadFloat2(&size), XMFLOAT4(static_cast<float>(tex_min.x), static_cast<float>(tex_min.y), uv_scale.x, uv_scale.y));
}

// Render a decal instance using the provided, internally-calculated parameters
void DecalRenderer::RenderDecalScreenInstance(const FXMVECTOR location, const FXMVECTOR size, const XMFLOAT4 & uv_shift_scale)
{
	// World matrix is a straightforward scale & translate in screen-space
	XMMATRIX world = XMMatrixMultiply(
		XMMatrixScalingFromVector(size),
		XMMatrixTranslationFromVector(XMVectorAdd(location, Game::Engine->ScreenSpaceAdjustment()))
	);

	// Add a new instance for rendering
	GetActiveRenderGroup().AddInstance(world, uv_shift_scale);
}

// End the frame
void DecalRenderer::EndFrame(void)
{

}


// Shutdown the renderer and release any allocated resources
void DecalRenderer::Shutdown(void)
{

}

// Destructor
DecalRenderer::~DecalRenderer(void)
{

}
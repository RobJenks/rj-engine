#include "DecalRenderingManager.h"
#include "CoreEngine.h"

// Constructor
DecalRenderingManager::DecalRenderingManager(void)
{
	// Make sure there is always an active render group so we can rely on !empty() 
	StartNewRenderGroup();
}

// Prepare the decal renderer for a new frame
void DecalRenderingManager::BeginFrame(void)
{
	// Clear all render group data ready for the current frame
	ClearAllRenderGroups();

	// Make sure there is always an active render group so we can always rely on !empty()
	StartNewRenderGroup();
}

// Set properties that are applied to all rendered decals
void DecalRenderingManager::SetTexture(const TextureDX11 * texture)
{
	// If no change is requested then exit immediately
	if (texture == GetActiveRenderGroup().GetTexture()) return;

	// Otherwise, start a new group (if necessary, i.e. if current group is already in use) and store the parameter
	GetNextAvailableRenderGroup().SetTexture(texture);
}

// Set properties that are applied to all rendered decals
void DecalRenderingManager::SetBaseColour(const XMFLOAT4 & colour)
{
	// If no change is requested then exit immediately
	if (Float4Equal(colour, GetActiveRenderGroup().GetBaseColour())) return;

	// Otherwise, start a new group (if necessary, i.e. if current group is already in use) and store the parameter
	GetNextAvailableRenderGroup().SetBaseColour(colour);
}

// Set properties that are applied to all rendered decals
void DecalRenderingManager::SetOutlineColour(const XMFLOAT4 & outline)
{
	// If no change is requested then exit immediately
	if (Float4Equal(outline, GetActiveRenderGroup().GetOutlineColour())) return;

	// Otherwise, start a new group (if necessary, i.e. if current group is already in use) and store the parameter
	GetNextAvailableRenderGroup().SetOutlineColour(outline);
}

// Set properties that are applied to all rendered decals
void DecalRenderingManager::SetOutlineWidthFactor(float widthFactor)
{
	// If no change is requested then exit immediately
	if (widthFactor == GetActiveRenderGroup().GetOutlineWidthFactor()) return;

	// Otherwise, start a new group (if necessary, i.e. if current group is already in use) and store the parameter
	GetNextAvailableRenderGroup().SetOutlineWidthFactor(widthFactor);
}


// Get the currently-active rendering group
DecalRenderingParams & DecalRenderingManager::GetActiveRenderGroup(void)
{
	return (m_rendergroups.back());
}

// Starts a new decal rendering group
void DecalRenderingManager::StartNewRenderGroup(void)
{
	m_rendergroups.push_back(DecalRenderingParams());
}

// Get a reference to the next available render group for use, creating a new one if necessary
DecalRenderingParams & DecalRenderingManager::GetNextAvailableRenderGroup(void)
{
	// Create a new group if the current group is already in use.  New group should
	// begin with same setup as the current group to account for previous calls to Set...
	if (GetActiveRenderGroup().IsInUse())
	{
		m_rendergroups.push_back(DecalRenderingParams());
		m_rendergroups.back().CloneData(m_rendergroups[m_rendergroups.size() - 2]);
	}

	return GetActiveRenderGroup();
}


// Clear all render groups and associated data
void DecalRenderingManager::ClearAllRenderGroups(void)
{
	m_rendergroups.clear();
}




// Render a decal to the given screen-space location
void DecalRenderingManager::RenderDecalScreen(const FXMVECTOR location, const FXMVECTOR size)
{
	RenderDecalScreenInstance(location, size, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));	// No shift, unit scale
}

// Render the specified subset of a decal to the given screen-space location
void DecalRenderingManager::RenderDecalScreen(const FXMVECTOR location, XMFLOAT2 size, UINTVECTOR2 tex_min, UINTVECTOR2 tex_max)
{
	// There must be an active texture assigned in order to make this call
	const TextureDX11 * texture = GetActiveRenderGroup().GetTexture();
	if (!texture) return;

	// Texture size = (100, 100)
	// Decal pos = (20, 50)
	// Decal size = (5, 10)
	// Decal pos% = (20/100, 50/100) = (0.2, 0.5)
	// Decal size% = (5/100, 10/100) = (0.05, 0.1)
	// In shader: UV = pos% + (UV * size%)
	XMFLOAT2 texsize = texture->Get2DSizeF();
	XMFLOAT2 decal_pos_pc(static_cast<float>(tex_min.x) / texsize.x, static_cast<float>(tex_min.y) / texsize.y);
	XMFLOAT2 decal_size = (tex_max - tex_min).ToFloat();
	XMFLOAT2 decal_size_pc(decal_size.x / texsize.x, decal_size.y / texsize.y);

	RenderDecalScreenInstance(location, XMLoadFloat2(&size), XMFLOAT4(decal_pos_pc.x, decal_pos_pc.y, decal_size_pc.x, decal_size_pc.y));
}

// Render a decal instance using the provided, internally-calculated parameters
void DecalRenderingManager::RenderDecalScreenInstance(const FXMVECTOR location, const FXMVECTOR size, const XMFLOAT4 & uv_shift_scale)
{
	// World matrix is a straightforward scale & translate in screen-space
	// TODO: Add rotation
	XMMATRIX world = XMMatrixMultiply(
		XMMatrixScalingFromVector(size),
		XMMatrixTranslationFromVector(Game::Engine->AdjustIntoLinearScreenSpace(location, size))
	);

	// Add a new instance for rendering
	GetActiveRenderGroup().AddInstance(world, uv_shift_scale);
}

// End the frame
void DecalRenderingManager::EndFrame(void)
{
}


// Shutdown the renderer and release any allocated resources
void DecalRenderingManager::Shutdown(void)
{

}

// Destructor
DecalRenderingManager::~DecalRenderingManager(void)
{

}


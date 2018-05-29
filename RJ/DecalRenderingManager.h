#pragma once

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "IntVector.h"
#include "DecalRenderingParams.h"
class TextureDX11;

class DecalRenderingManager
{
public:

	typedef std::vector<DecalRenderingParams> DecalRenderingQueue;

	// Constructor
	DecalRenderingManager(void);

	// Prepare the decal renderer for a new frame
	void									BeginFrame(void);

	// Set properties that are applied to all rendered decals
	void									SetTexture(const TextureDX11 * texture);
	void									SetBaseColour(const XMFLOAT4 & colour);
	void									SetOutlineColour(const XMFLOAT4 & outline);
	void									SetOutlineWidthFactor(float widthFactor);

	// Render a decal to the given screen-space location
	void									RenderDecalScreen(const FXMVECTOR location, const FXMVECTOR size);
	void									RenderDecalScreen(const FXMVECTOR location, XMFLOAT2 size, UINTVECTOR2 tex_min, UINTVECTOR2 tex_max);

	// End the frame
	void									EndFrame(void);

	// Retrieve the set of decal rendering data that is queued for the render device this frame
	CMPINLINE const DecalRenderingQueue &	GetQueuedRenderingData(void) const { return m_rendergroups; }

	// Shutdown the renderer and release any allocated resources
	void									Shutdown(void);

	// Destructor
	~DecalRenderingManager(void);


private:

	// Render a decal instance using the provided, internally-calculated parameters
	void									RenderDecalScreenInstance(const FXMVECTOR location, const FXMVECTOR size, const XMFLOAT4 & uv_shift_scale);

	// Get the currently-active rendering group
	DecalRenderingParams &					GetActiveRenderGroup(void);

	// Starts a new decal rendering group
	void									StartNewRenderGroup(void);

	// Get a reference to the next available render group for use, creating a new one if necessary
	DecalRenderingParams &					GetNextAvailableRenderGroup(void);

	// Clear all render groups and associated data
	void									ClearAllRenderGroups(void);


private:

	// Collection of decal rendering params, which group similar rendering requests together for render-time efficiency
	DecalRenderingQueue						m_rendergroups;


};
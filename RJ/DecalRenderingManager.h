#pragma once

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "FastMath.h"
#include "IntVector.h"
#include "DecalRenderingMode.h"
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
	void									SetSmoothingFactor(float factor);

	// Render a decal to the given location
	void									RenderDecal(DecalRenderingMode mode, const FXMVECTOR location, const FXMVECTOR size, const FXMVECTOR orientation = ID_QUATERNION);
	void									RenderDecal(DecalRenderingMode mode, const FXMVECTOR location, XMFLOAT2 size, UINTVECTOR2 tex_min, UINTVECTOR2 tex_max, const FXMVECTOR orientation = ID_QUATERNION);

	// End the frame
	void									EndFrame(void);

	// Retrieve the set of decal rendering data that is queued for the render device this frame
	CMPINLINE const DecalRenderingQueue &	GetQueuedRenderingData(void) const { return m_rendergroups; }

	// Shutdown the renderer and release any allocated resources
	void									Shutdown(void);

	// Destructor
	~DecalRenderingManager(void);


private:

	// Set rendering mode; this is not public like other properties since it is set by the relevant render method in this class
	void									SetRenderingMode(DecalRenderingMode mode);

	// Render a decal instance using the provided, internally-calculated parameters
	void									RenderDecalInstance(DecalRenderingMode mode, const FXMVECTOR location, const FXMVECTOR orientation, const FXMVECTOR size, const XMFLOAT4 & uv_shift_scale);


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
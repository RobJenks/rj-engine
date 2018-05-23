#pragma once

#include <vector>
#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "RM_Instance.h"
class TextureDX11;


class DecalRenderingParams
{
public:

	// Constructor
	DecalRenderingParams(void);
	DecalRenderingParams(const TextureDX11 * texture, const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour);

	// Return render group params
	CMPINLINE const TextureDX11 *			GetTexture(void) const { return m_texture; }
	CMPINLINE XMFLOAT4						GetBaseColour(void) const { return m_basecolour; }
	CMPINLINE XMFLOAT4						GetOutlineColour(void) const { return m_outlinecolour; }

	// Update render group params
	CMPINLINE void							SetTexture(const TextureDX11 * texture) { m_texture = texture; }
	CMPINLINE void							SetBaseColour(const XMFLOAT4 & basecolour) { m_basecolour = basecolour; }
	CMPINLINE void							SetOutlineColour(const XMFLOAT4 & outlinecolour) { m_outlinecolour = outlinecolour; }

	// Add a new instance for rendering
	void									AddInstance(const FXMMATRIX world, const XMFLOAT4 & uv_shift_scale);

	// Return a reference to the instance data calculated for rendering
	CMPINLINE const std::vector<RM_Instance> &	GetQueuedInstanceData(void) const { return m_instances; }

	// Indicates whether this group is currently 'in use', i.e. if it has any instances assigned
	CMPINLINE bool							IsInUse(void) const { return (!m_instances.empty()); }





	// Destructor
	~DecalRenderingParams(void);

private:

	// Properties common to all instances in this set
	const TextureDX11 *						m_texture;
	XMFLOAT4								m_basecolour;
	XMFLOAT4								m_outlinecolour;

	// Passed in RM_Instance instance params
	// - matrix world		- determines pos & size of individual decal
	// - float2 uv_offset	- shift uv coords to allow selection of e.g. one glyph in a font sheet
	// - float2 uv_scale	- scale uv coords, calculated based on obj & desired texture range, to allow any combination of texture subsets and model size
	std::vector<RM_Instance>				m_instances;


	

};
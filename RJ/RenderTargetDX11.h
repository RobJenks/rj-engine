#pragma once

#include <array>
#include "FastMath.h"
#include "Rendering.h"
#include "RenderTarget.h"
#include "ClearFlags.h"
class TextureDX11;
class StructuredBufferDX11;

class RenderTargetDX11 : public RenderTarget
{
public:

	typedef std::array<TextureDX11*, static_cast<uint8_t>(AttachmentPoint::NumAttachmentPoints)>			TextureList;
	typedef std::array<StructuredBufferDX11*, Rendering::MaxRenderTargets>									StructuredBufferList;

	RenderTargetDX11(void);
	~RenderTargetDX11();

	void AttachTexture(AttachmentPoint attachment, TextureDX11 *texture);
	TextureDX11 * GetTexture(AttachmentPoint attachment);
	void Clear(AttachmentPoint attachemnt, ClearFlags clearFlags = ClearFlags::All, const XMFLOAT4 & color = NULL_FLOAT4, float depth = 1.0f, uint8_t stencil = 0);
	void Clear(ClearFlags clearFlags = ClearFlags::All, const XMFLOAT4 & color = NULL_FLOAT4, float depth = 1.0f, uint8_t stencil = 0);
	void GenerateMipMaps(void);
	void AttachStructuredBuffer(uint8_t slot, StructuredBufferDX11 *rwBuffer);
	StructuredBufferDX11 * GetStructuredBuffer(uint8_t slot);
	void Resize(uint16_t width, uint16_t height);

	void Bind();
	void Unbind();

	bool IsValid() const;

private:

	TextureList						m_textures;
	StructuredBufferList			m_structured_buffers;

	uint16_t						m_width;	// Width in pixels of textures associated with this RT
	uint16_t						m_height;	// Height in pixels of textures associated with this RT


	// Compiled resources, recreated when RT contents change
	ID3D11RenderTargetView *		m_compiled_rtvs[Rendering::MaxRenderTargets];
	UINT							m_num_compiled_rtvs;
	ID3D11UnorderedAccessView *		m_compiled_uavs[Rendering::MaxRenderTargets];
	UINT							m_num_compiled_uavs;
	UINT							m_compiled_uav_start_slot;
	ID3D11DepthStencilView *		m_compiled_dsv;

	// Flag indicating whether the RT contents have changed and need to be recompiled.  Also performs validation 
	// of the RT contents against API requirements
	bool							m_isdirty;
};
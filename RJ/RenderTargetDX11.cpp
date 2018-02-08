#include "RenderTargetDX11.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "TextureDX11.h"
#include "StructuredBufferDX11.h"


RenderTargetDX11::RenderTargetDX11(void)
	: m_width(0)
	, m_height(0)
	, m_num_compiled_rtvs(0U)
	, m_num_compiled_uavs(0U)
	, m_compiled_uav_start_slot(0U)
	, m_compiled_dsv(NULL)
	, m_isdirty(false)
{
	for (size_t i = 0; i < static_cast<size_t>(AttachmentPoint::NumAttachmentPoints); ++i)
	{
		m_textures[i] = NULL;
	}

	for (size_t i = 0; i < Rendering::MaxRenderTargets; ++i)
	{
		m_structured_buffers[i] = NULL;
		m_compiled_rtvs[i] = NULL;
		m_compiled_uavs[i] = NULL;
	}
}

RenderTargetDX11::~RenderTargetDX11()
{
}

void RenderTargetDX11::AttachTexture(AttachmentPoint attachment, TextureDX11 *texture)
{
	m_textures[static_cast<uint8_t>(attachment)] = texture;

	// Next time the render target is bound, check that it is valid.
	m_isdirty = true;
}

TextureDX11 * RenderTargetDX11::GetTexture(AttachmentPoint attachment)
{
	return m_textures[static_cast<uint8_t>(attachment)];
}


void RenderTargetDX11::Clear(AttachmentPoint attachment, ClearFlags clearFlags, const XMFLOAT4 & color, float depth, uint8_t stencil)
{
	TextureDX11 *texture = m_textures[static_cast<uint8_t>(attachment)];
	if (texture)
	{
		texture->Clear(clearFlags, ValuePtr(color), depth, stencil);
	}
}

void RenderTargetDX11::Clear(ClearFlags clearFlags, const XMFLOAT4 & color, float depth, uint8_t stencil)
{
	TextureDX11 *texture;
	for (uint8_t i = 0; i < (uint8_t)AttachmentPoint::NumAttachmentPoints; ++i)
	{
		texture = m_textures[i];
		if (texture)
		{
			texture->Clear(clearFlags, ValuePtr(color), depth, stencil);
		}
	}
}

void RenderTargetDX11::GenerateMipMaps()
{
	for (auto * texture : m_textures)
	{
		if (texture)
		{
			texture->GenerateMipMaps();
		}
	}
}

void RenderTargetDX11::AttachStructuredBuffer(uint8_t slot, StructuredBufferDX11 * rwBuffer)
{
	m_structured_buffers[slot] = rwBuffer;

	// Next time the render target is bound, check that it is valid.
	m_isdirty = true;
}

StructuredBufferDX11 * RenderTargetDX11::GetStructuredBuffer(uint8_t slot)
{
	if (slot < Rendering::MaxRenderTargets)
	{
		return m_structured_buffers[slot];
	}

	Game::Log << LOG_ERROR << "Render target request for invalid structured buffer instance " << slot << "\n";
	return NULL;	// This will likely fail with NPE very shortly afterwards
}


void RenderTargetDX11::Resize(uint16_t width, uint16_t height)
{
	if (m_width != width || m_height != height)
	{
		m_width = max(width, (uint16_t)1);
		m_height = max(height, (uint16_t)1);

		// Resize the attached textures.
		for (auto * texture : m_textures)
		{
			if (texture)
			{
				texture->Resize(m_width, m_height);
			}
		}
	}

	m_isdirty = true;
}

void RenderTargetDX11::Bind()
{
	if (m_isdirty)
	{
		if (!IsValid())
		{
			Game::Log << LOG_ERROR << ("Invalid render target\n");
		}

		// Build compiled RTV / UAV array for submission
		m_num_compiled_rtvs = 0U;
		m_num_compiled_uavs = 0U;
		for (UINT i = 0; i < Rendering::MaxRenderTargets; ++i)
		{
			if (m_textures[i])				m_compiled_rtvs[m_num_compiled_rtvs++] = m_textures[i]->GetRenderTargetView();
			if (m_structured_buffers[i])	m_compiled_uavs[m_num_compiled_uavs++] = m_structured_buffers[i]->GetUnorderedAccessView();
		}
		m_compiled_uav_start_slot = m_num_compiled_rtvs;

		// Get pointer to the compiled DSV for this RV
		if ( m_textures[(uint8_t)AttachmentPoint::Depth] )
		{
			m_compiled_dsv = m_textures[(uint8_t)AttachmentPoint::Depth]->GetDepthStencilView();
		}
		else if (m_textures[(uint8_t)AttachmentPoint::DepthStencil])
		{
			m_compiled_dsv = m_textures[(uint8_t)AttachmentPoint::DepthStencil]->GetDepthStencilView();
		}
		else
		{
			m_compiled_dsv = NULL;
		}

		m_isdirty = false;
	}

	Game::Engine->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(m_num_compiled_rtvs, m_compiled_rtvs, m_compiled_dsv, 
		m_compiled_uav_start_slot, m_num_compiled_uavs, m_compiled_uavs, nullptr);
}

void RenderTargetDX11::Unbind()
{
	Game::Engine->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 0, nullptr, nullptr);
}

bool RenderTargetDX11::IsValid() const
{
	UINT numRTV = 0;
	int width = -1;
	int height = -1;

	for (auto * texture : m_textures)
	{
		if (texture)
		{
			if (texture->GetRenderTargetView()) ++numRTV;

			if (width == -1 || height == -1)
			{
				width = texture->GetWidth();
				height = texture->GetHeight();
			}
			else
			{
				if (texture->GetWidth() != width || texture->GetHeight() != height)
				{
					return false;
				}
			}
		}
	}

	UINT numUAV = 0;
	for (auto rwBuffer : m_structured_buffers)
	{
		if (rwBuffer)
		{
			++numUAV;
		}
	}

	if (numRTV + numUAV > Rendering::MaxRenderTargets)
	{
		return false;
	}

	return true;
}

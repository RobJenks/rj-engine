#include "TextureDX11.h"
#include "Logging.h"
#include "CoreEngine.h"

// Default constructor; empty texture
TextureDX11::TextureDX11(void)
	:
	  m_texture1d(NULL), m_texture2d(NULL), m_texture3d(NULL)
	, m_srv(NULL), m_uav(NULL), m_rendertarget_view(NULL), m_depthstencil_view(NULL)
	, m_width(0)
	, m_height(0)
	, m_numslices(0)
	, m_TextureResourceFormatSupport(0)
	, m_DepthStencilViewFormatSupport(0)
	, m_ShaderResourceViewFormatSupport(0)
	, m_RenderTargetViewFormatSupport(0)
	, m_cpuaccess(CPUGraphicsResourceAccess::None)
	, m_isdynamic(false)
	, m_is_uav(false)
	, m_TextureResourceFormat(DXGI_FORMAT_UNKNOWN)
	, m_DepthStencilViewFormat(DXGI_FORMAT_UNKNOWN)
	, m_ShaderResourceViewFormat(DXGI_FORMAT_UNKNOWN)
	, m_RenderTargetViewFormat(DXGI_FORMAT_UNKNOWN)
	, m_generate_mipmaps(false)
	, m_bpp(0)
	, m_pitch(0)
	, m_istransparent(false)
{
}

// 1D Texture
TextureDX11::TextureDX11(uint16_t width, uint16_t slices, const TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool bUAV)
	: 
	  m_texture1d(NULL), m_texture2d(NULL), m_texture3d(NULL)
	, m_srv(NULL), m_uav(NULL), m_rendertarget_view(NULL), m_depthstencil_view(NULL)
	, m_width(0)
	, m_height(1)
	, m_TextureResourceFormatSupport(0)
	, m_cpuaccess(cpuAccess)
	, m_generate_mipmaps(false)
	, m_bpp(0)
	, m_pitch(0)
	, m_istransparent(false)
{
	ID3D11Device2 *device = Game::Engine->GetDevice();
	m_numslices = max(slices, (uint16_t)1);

	m_dimension = Dimension::Texture1D;
	if (m_numslices > 1)
	{
		m_dimension = Dimension::Texture1DArray;
	}

	// Translate to DXGI format
	DXGI_FORMAT dxgiFormat = TranslateFormat(format);
	m_SampleDesc = GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = GetUAVFormat(dxgiFormat);

	m_bpp = GetBPP(m_TextureResourceFormat);

	// Query for texture format support.
	if (FAILED(device->CheckFormatSupport(m_TextureResourceFormat, &m_TextureResourceFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query texture resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_DepthStencilViewFormat, &m_DepthStencilViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query depth/stencil view format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_ShaderResourceViewFormat, &m_ShaderResourceViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query shader resource view format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_RenderTargetViewFormat, &m_RenderTargetViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target view format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_UnorderedAccessViewFormat, &m_UnorderedAccessViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query unordered access view format support.");
	}
	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE1D) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for 1D textures");
	}

	// Can the texture be dynamically modified on the CPU?
	m_isdynamic = (int)m_cpuaccess != 0 && (m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE) != 0;
	// Can mipmaps be automatically generated for this texture format?
	m_generate_mipmaps = !m_isdynamic && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0;
	// Are UAVs supported?
	m_is_uav = bUAV && (m_UnorderedAccessViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_LOAD) != 0;

	// Resize the texture to the requested dimension.
	Resize(width);
}

// 2D Texture
TextureDX11::TextureDX11(uint16_t width, uint16_t height, uint16_t slices, const TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool bUAV)
	: 
	  m_texture1d(NULL), m_texture2d(NULL), m_texture3d(NULL)
	, m_srv(NULL), m_uav(NULL), m_rendertarget_view(NULL), m_depthstencil_view(NULL)
	, m_width(0)
	, m_height(0)
	, m_bpp(0)
	, m_format(format)
	, m_cpuaccess(cpuAccess)
	, m_generate_mipmaps(false)
	, m_istransparent(true)
{
	ID3D11Device2 *device = Game::Engine->GetDevice();

	m_numslices = max(slices, (uint16_t)1);

	m_dimension = Dimension::Texture2D;
	if (m_numslices > 1)
	{
		m_dimension = Dimension::Texture2DArray;
	}

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TranslateFormat(format);
	m_SampleDesc = GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format).
	m_format = TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats.
	m_TextureResourceFormat = GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats.
	m_DepthStencilViewFormat = GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats.
	m_ShaderResourceViewFormat = GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats.
	m_RenderTargetViewFormat = GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = GetUAVFormat(dxgiFormat);

	m_bpp = GetBPP(m_TextureResourceFormat);

	// Query for texture format support.
	if (FAILED(device->CheckFormatSupport(m_TextureResourceFormat, &m_TextureResourceFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query texture resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_DepthStencilViewFormat, &m_DepthStencilViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query depth/stencil format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_ShaderResourceViewFormat, &m_ShaderResourceViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query shader resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_RenderTargetViewFormat, &m_RenderTargetViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_UnorderedAccessViewFormat, &m_UnorderedAccessViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}
	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for 2D textures");
	}
	// Can the texture be dynamically modified on the CPU?
	m_isdynamic = (int)m_cpuaccess != 0 && (m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE) != 0;
	// Can mipmaps be automatically generated for this texture format?
	m_generate_mipmaps = !m_isdynamic && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0;
	// Are UAVs supported?
	m_is_uav = bUAV && (m_UnorderedAccessViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_LOAD) != 0;

	Resize(width, height);
}

// 3D Texture
TextureDX11::TextureDX11(Texture3DConstructor, uint16_t width, uint16_t height, uint16_t depth, const TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool bUAV)
	: 
	  m_texture1d(NULL), m_texture2d(NULL), m_texture3d(NULL)
	, m_srv(NULL), m_uav(NULL), m_rendertarget_view(NULL), m_depthstencil_view(NULL)
{
	ID3D11Device2 *device = Game::Engine->GetDevice();

	m_numslices = max(depth, (uint16_t)1);
	m_dimension = Dimension::Texture3D;

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TranslateFormat(format);
	m_SampleDesc = GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = GetUAVFormat(dxgiFormat);

	m_bpp = GetBPP(m_TextureResourceFormat);

	// Query for texture format support.
	if (FAILED(device->CheckFormatSupport(m_TextureResourceFormat, &m_TextureResourceFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query texture resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_DepthStencilViewFormat, &m_DepthStencilViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query depth/stencil format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_ShaderResourceViewFormat, &m_ShaderResourceViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query shader resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_RenderTargetViewFormat, &m_RenderTargetViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_UnorderedAccessViewFormat, &m_UnorderedAccessViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}

	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE3D) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for 3D textures");
	}
	// Can the texture be dynamically modified on the CPU?
	m_isdynamic = (int)m_cpuaccess != 0 && (m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE) != 0;
	// Can mipmaps be automatically generated for this texture format?
	m_generate_mipmaps = !m_isdynamic && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0;
	// Are UAVs supported?
	m_is_uav = bUAV && (m_UnorderedAccessViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_LOAD) != 0;

}

// CUBE Texture
TextureDX11::TextureDX11(CubeMapConstructor, uint16_t size, uint16_t count, const TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool bUAV)
	:
	  m_texture1d(NULL), m_texture2d(NULL), m_texture3d(NULL)
	, m_srv(NULL), m_uav(NULL), m_rendertarget_view(NULL), m_depthstencil_view(NULL)
{
	ID3D11Device2 *device = Game::Engine->GetDevice();

	m_dimension = Texture::Dimension::TextureCube;
	m_width = m_height = size;

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TranslateFormat(format);
	m_SampleDesc = GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = GetUAVFormat(dxgiFormat);

	// Query for texture format support.
	if (FAILED(device->CheckFormatSupport(m_TextureResourceFormat, &m_TextureResourceFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query texture resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_DepthStencilViewFormat, &m_DepthStencilViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query depth/stencil format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_ShaderResourceViewFormat, &m_ShaderResourceViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query shader resource format support.");
	}
	if (FAILED(device->CheckFormatSupport(m_RenderTargetViewFormat, &m_RenderTargetViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}
	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURECUBE) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for cubemap textures");
	}
	if (FAILED(device->CheckFormatSupport(m_UnorderedAccessViewFormat, &m_UnorderedAccessViewFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query render target format support.");
	}

	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURECUBE) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for cube textures");
	}

	// Can the texture be dynamically modified on the CPU?
	m_isdynamic = ((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Write) != 0 && (m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE) != 0;
	// Can mipmaps be automatically generated for this texture format?
	m_generate_mipmaps = !m_isdynamic && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0; // && ( m_RenderTargetViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN ) != 0;
																													 // Are UAVs supported?
	m_is_uav = bUAV && (m_UnorderedAccessViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_LOAD) != 0;

}









// Reports a more detailed texture error with format information extracted from the texture resource
void LogTextureFormatError(const Texture::TextureFormat& format, const std::string& file, int line, const std::string& function, const std::string& message)
{
	std::stringstream ss;
	ss << message << std::endl;
	ss << "Components: ";
	switch (format.Components)
	{
	case Texture::Components::R:
		ss << "R" << std::endl;
		break;
	case Texture::Components::RG:
		ss << "RG" << std::endl;
		break;
	case Texture::Components::RGB:
		ss << "RGB" << std::endl;
		break;
	case Texture::Components::RGBA:
		ss << "RGBA" << std::endl;
		break;
	case Texture::Components::Depth:
		ss << "Depth" << std::endl;
		break;
	case Texture::Components::DepthStencil:
		ss << "DepthStencil" << std::endl;
		break;
	default:
		ss << "Unknown" << std::endl;
		break;
	}

	ss << "Type:";
	switch (format.Type)
	{
	case Texture::Type::Typeless:
		ss << "Typeless" << std::endl;
		break;
	case Texture::Type::UnsignedNormalized:
		ss << "UnsignedNormalized" << std::endl;
		break;
	case Texture::Type::SignedNormalized:
		ss << "SignedNormalized" << std::endl;
		break;
	case Texture::Type::Float:
		ss << "Float" << std::endl;
		break;
	case Texture::Type::UnsignedInteger:
		ss << "UnsignedInteger" << std::endl;
		break;
	case Texture::Type::SignedInteger:
		ss << "SignedInteger" << std::endl;
		break;
	default:
		ss << "Unknown" << std::endl;
		break;
	}

	ss << "RedBits:     " << (int32_t)format.RedBits << std::endl;
	ss << "GreenBits:   " << (int32_t)format.GreenBits << std::endl;
	ss << "BlueBits:    " << (int32_t)format.BlueBits << std::endl;
	ss << "AlphaBits:   " << (int32_t)format.AlphaBits << std::endl;
	ss << "DepthBits:   " << (int32_t)format.DepthBits << std::endl;
	ss << "StencilBits: " << (int32_t)format.StencilBits << std::endl;
	ss << "Num Samples: " << (int32_t)format.NumSamples << std::endl;

	Game::Log << LOG_ERROR << ss.str() << " [" << file << ":" << line << " (" << function << ")]\n";
}




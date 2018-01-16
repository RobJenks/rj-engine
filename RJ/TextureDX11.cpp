#include <filesystem>
#include <unordered_map>
#include "TextureDX11.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "FreeImage.h"		// External dependency; used to read image/texture metadata and properties
#include "TextureDX11Util.h"

// TODO: VS2017 is still implementing as exp branch; convert to std library once available
namespace fs = std::experimental::filesystem;

// Initialise static fields
const FLOAT * float4_zero = new FLOAT[4] { 0 };
ID3D11ShaderResourceView * const null_srv[1] = { nullptr };
ID3D11UnorderedAccessView * const null_uav[1] = { nullptr };

// Global collection of unique texture resources
std::unordered_map<std::string, TextureDX11*> TextureDX11::TextureResources;


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
	auto device = Game::Engine->GetDevice();
	m_numslices = max(slices, (uint16_t)1);

	m_dimension = Dimension::Texture1D;
	if (m_numslices > 1)
	{
		m_dimension = Dimension::Texture1DArray;
	}

	// Translate to DXGI format
	DXGI_FORMAT dxgiFormat = TextureDX11Util::TranslateFormat(format);
	m_SampleDesc = TextureDX11Util::GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TextureDX11Util::TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = TextureDX11Util::GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = TextureDX11Util::GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = TextureDX11Util::GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = TextureDX11Util::GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = TextureDX11Util::GetUAVFormat(dxgiFormat);

	m_bpp = TextureDX11Util::GetBPP(m_TextureResourceFormat);

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
	auto device = Game::Engine->GetDevice();

	m_numslices = max(slices, (uint16_t)1);

	m_dimension = Dimension::Texture2D;
	if (m_numslices > 1)
	{
		m_dimension = Dimension::Texture2DArray;
	}

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TextureDX11Util::TranslateFormat(format);
	m_SampleDesc = TextureDX11Util::GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format).
	m_format = TextureDX11Util::TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats.
	m_TextureResourceFormat = TextureDX11Util::GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats.
	m_DepthStencilViewFormat = TextureDX11Util::GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats.
	m_ShaderResourceViewFormat = TextureDX11Util::GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats.
	m_RenderTargetViewFormat = TextureDX11Util::GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = TextureDX11Util::GetUAVFormat(dxgiFormat);

	m_bpp = TextureDX11Util::GetBPP(m_TextureResourceFormat);

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
	auto device = Game::Engine->GetDevice();

	m_numslices = max(depth, (uint16_t)1);
	m_dimension = Dimension::Texture3D;

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TextureDX11Util::TranslateFormat(format);
	m_SampleDesc = TextureDX11Util::GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TextureDX11Util::TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = TextureDX11Util::GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = TextureDX11Util::GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = TextureDX11Util::GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = TextureDX11Util::GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = TextureDX11Util::GetUAVFormat(dxgiFormat);

	m_bpp = TextureDX11Util::GetBPP(m_TextureResourceFormat);

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
	auto device = Game::Engine->GetDevice();

	m_dimension = Texture::Dimension::TextureCube;
	m_width = m_height = size;

	// Translate to DXGI format.
	DXGI_FORMAT dxgiFormat = TextureDX11Util::TranslateFormat(format);
	m_SampleDesc = TextureDX11Util::GetSupportedSampleCount(dxgiFormat, format.NumSamples);

	// Translate back to original format (for best match format)
	m_format = TextureDX11Util::TranslateFormat(dxgiFormat, format.NumSamples);

	// Convert Depth/Stencil formats to typeless texture resource formats
	m_TextureResourceFormat = TextureDX11Util::GetTextureFormat(dxgiFormat);
	// Convert typeless formats to Depth/Stencil view formats
	m_DepthStencilViewFormat = TextureDX11Util::GetDSVFormat(dxgiFormat);
	// Convert depth/stencil and typeless formats to Shader Resource View formats
	m_ShaderResourceViewFormat = TextureDX11Util::GetSRVFormat(dxgiFormat);
	// Convert typeless formats to Render Target View formats
	m_RenderTargetViewFormat = TextureDX11Util::GetRTVFormat(dxgiFormat);
	// Convert typeless format to Unordered Access View formats.
	m_UnorderedAccessViewFormat = TextureDX11Util::GetUAVFormat(dxgiFormat);

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

// Load a texture based on the supplied dimension type
bool TextureDX11::LoadTexture(const std::wstring & fileName, Texture::Dimension dimension)
{
	switch (dimension)
	{
		case Texture::Dimension::Texture2D:			return LoadTexture2D(fileName);
		case Texture::Dimension::TextureCube:		return LoadTextureCube(fileName);
		
		default:

			Game::Log << LOG_WARN << "Cannot load texture data for \"" << ConvertWStringToString(fileName) << "\"; unsupported dimension type " << (int)dimension << "\n";
			return false;
	}
}

bool TextureDX11::LoadTexture2D(const std::wstring& fileName)
{
	auto device = Game::Engine->GetDevice();
	auto devicecontext = Game::Engine->GetDeviceContext();

	fs::path filePath(fileName);
	if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
	{
		Game::Log << LOG_ERROR << ("Could not load texture: " + filePath.string());
		return false;
	}

	m_filename = fileName;

	// Try to determine the file type from the image file.
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(filePath.c_str());
	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilenameU(filePath.c_str());
	}

	if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
	{
		Game::Log << LOG_ERROR << ("Unknown file format: " + filePath.string());
		return false;
	}

	FIBITMAP* dib = FreeImage_LoadU(fif, filePath.c_str());
	if (dib == nullptr || FreeImage_HasPixels(dib) == FALSE)
	{
		Game::Log << LOG_ERROR << ("Failed to load image: " + filePath.string());
		return false;
	}

	//// Check to see if we need to flip the image
	//for ( int model = 0; model < FIMD_EXIF_RAW + 1; model++ )
	//{
	//    PrintMetaData( (FREE_IMAGE_MDMODEL)model, dib );
	//}

	m_bpp = FreeImage_GetBPP(dib);
	FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(dib);

	// Check to see if the texture has an alpha channel.
	m_istransparent = (FreeImage_IsTransparent(dib) == TRUE);

	switch (m_bpp)
	{
	case 8:
	{
		switch (imageType)
		{
		case FIT_BITMAP:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R8_UNORM;
		}
		break;
		default:
		{
			Game::Log << LOG_ERROR << ("Unknown image format.");
		}
		break;
		}
	}
	break;
	case 16:
	{
		switch (imageType)
		{
		case FIT_BITMAP:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R8G8_UNORM;
		}
		break;
		case FIT_UINT16:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R16_UINT;
		}
		break;
		case FIT_INT16:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R16_SINT;
		}
		break;
		default:
		{
			Game::Log << LOG_ERROR << ("Unknown image format.");
		}
		break;
		}
	}
	break;
	case 32:
	{
		switch (imageType)
		{
		case FIT_BITMAP:
		{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
			m_TextureResourceFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
#else
			m_TextureResourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif
		}
		break;
		case FIT_FLOAT:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R32_FLOAT;
		}
		break;
		case FIT_INT32:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R32_SINT;
		}
		break;
		case FIT_UINT32:
		{
			m_TextureResourceFormat = DXGI_FORMAT_R32_UINT;
		}
		break;
		default:
		{
			Game::Log << LOG_ERROR << ("Unknown image format.");
		}
		break;
		}
	}
	break;
	default:
	{
		FIBITMAP* dib32 = FreeImage_ConvertTo32Bits(dib);

		// Unload the original image.
		FreeImage_Unload(dib);

		dib = dib32;

		// Update pixel bit depth (should be 32 now if it wasn't before).
		m_bpp = FreeImage_GetBPP(dib);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		m_TextureResourceFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
#else
		m_TextureResourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif
	}
	break;
	}

	m_dimension = Texture::Dimension::Texture2D;
	m_width  = FreeImage_GetWidth(dib);
	m_height = FreeImage_GetHeight(dib);
	m_numslices = 1;
	m_pitch = FreeImage_GetPitch(dib);

	m_ShaderResourceViewFormat = m_RenderTargetViewFormat = m_TextureResourceFormat;
	m_SampleDesc = TextureDX11Util::GetSupportedSampleCount(m_TextureResourceFormat, 1);

	if (FAILED(device->CheckFormatSupport(m_TextureResourceFormat, &m_TextureResourceFormatSupport)))
	{
		Game::Log << LOG_ERROR << ("Failed to query format support.");
	}
	if ((m_TextureResourceFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) == 0)
	{
		ReportTextureFormatError(m_format, "Unsupported texture format for 2D textures.");
		return false;
	}

	m_ShaderResourceViewFormatSupport = m_RenderTargetViewFormatSupport = m_TextureResourceFormatSupport;

	// Can mipmaps be automatically generated for this texture format?
	m_generate_mipmaps = !m_isdynamic && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0;

	// Load the texture data into a GPU texture.
	D3D11_TEXTURE2D_DESC textureDesc = { 0 };

	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = m_generate_mipmaps ? 0 : 1;
	textureDesc.ArraySize = m_numslices;
	textureDesc.Format = m_TextureResourceFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	if ((m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) != 0)
	{
		textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if ((m_RenderTargetViewFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = m_generate_mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	BYTE* textureData = FreeImage_GetBits(dib);

	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = textureData;
	subresourceData.SysMemPitch = m_pitch;
	subresourceData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, m_generate_mipmaps ? nullptr : &subresourceData, &m_texture2d)))
	{
		Game::Log << LOG_ERROR << ("Failed to create texture.");
		return false;
	}

	// Create a Shader resource view for the texture.
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;

	resourceViewDesc.Format = m_ShaderResourceViewFormat;
	resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels = m_generate_mipmaps ? -1 : 1;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;

	if (FAILED(device->CreateShaderResourceView(m_texture2d, &resourceViewDesc, &m_srv)))
	{
		Game::Log << LOG_ERROR << ("Failed to create texture resource view.");
		return false;
	}

	// From DirectXTK (28/05/2015) @see https://directxtk.codeplex.com/
	if (m_generate_mipmaps)
	{
		devicecontext->UpdateSubresource(m_texture2d, 0, nullptr, textureData, m_pitch, 0);
		devicecontext->GenerateMips(m_srv);
	}

	// Unload the texture (it should now be on the GPU anyway).
	FreeImage_Unload(dib);

	return true;
}

bool TextureDX11::LoadTextureCube(const std::wstring& fileName)
{
	auto device = Game::Engine->GetDevice();
	auto devicecontext = Game::Engine->GetDeviceContext();

	fs::path filePath(fileName);
	if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
	{
		Game::Log << LOG_ERROR << ("Could not load texture: " + filePath.string());
		return false;
	}

	m_filename = fileName;
	
	// Try to determine the file type from the image file.
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(filePath.c_str());
	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilenameU(filePath.c_str());
	}

	if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
	{
		Game::Log << LOG_ERROR << ("Unknown file format: " + filePath.string());
		return false;
	}

	FIMULTIBITMAP* dib = FreeImage_OpenMultiBitmap(fif, filePath.string().c_str(), FALSE, TRUE, TRUE);
	if (dib == nullptr || FreeImage_GetPageCount(dib) == 0)
	{
		Game::Log << LOG_ERROR << ("Failed to load image: " + filePath.string());
		return false;
	}

	int pageCount = FreeImage_GetPageCount(dib);

	// TODO: DDS cubemap loading with FreeImage?

	return false;
}

void TextureDX11::GenerateMipMaps()
{
	if (m_generate_mipmaps && m_srv)
	{
		Game::Engine->GetDeviceContext()->GenerateMips(m_srv);
	}
}

TextureDX11 * TextureDX11::GetFace(CubeFace face)
{
	// Simply return a pointer to this texture since 3D/cube textures are not yet fully-supported
	return this;
}

TextureDX11 * TextureDX11::GetSlice(unsigned int slice)
{
	// Simply return a pointer to this texture since 3D/cube textures are not yet fully-supported
	return this;
}

uint16_t TextureDX11::GetWidth(void) const
{
	return m_width;
}

uint16_t TextureDX11::GetHeight(void) const
{
	return m_height;
}

uint16_t TextureDX11::GetDepth(void) const
{
	return m_numslices;
}

// Return the image size in aggregate
INTVECTOR2 TextureDX11::Get2DSize(void) const
{
	return INTVECTOR2(m_width, m_height);
}

// Return the image size in aggregate
INTVECTOR3 TextureDX11::Get3DSize(void) const
{
	return INTVECTOR3(m_width, m_height, m_numslices);
}

uint8_t TextureDX11::GetBPP(void) const
{
	return m_bpp;
}

bool TextureDX11::IsTransparent(void) const
{
	return m_istransparent;
}


void TextureDX11::Resize1D(uint16_t width)
{
	auto device = Game::Engine->GetDevice();
	auto devicecontext = Game::Engine->GetDeviceContext();

	if (m_width != width)
	{
		m_texture1d = NULL;
		m_srv = NULL;
		m_rendertarget_view = NULL;
		m_depthstencil_view = NULL;
		m_uav = NULL;

		m_width = max(width, (uint16_t)1);

		D3D11_TEXTURE1D_DESC textureDesc = { 0 };

		textureDesc.Width = m_width;
		textureDesc.ArraySize = m_numslices;
		textureDesc.Format = m_TextureResourceFormat;
		textureDesc.MipLevels = 1;

		if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Read) != 0)
		{
			textureDesc.Usage = D3D11_USAGE_STAGING;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		}
		else if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Write) != 0)
		{
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.CPUAccessFlags = 0;
		}

		if (!m_is_uav && !m_isdynamic && (m_DepthStencilViewFormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) != 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		if (!m_isdynamic && (m_RenderTargetViewFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Read) == 0 && (m_ShaderResourceViewFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) != 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (m_is_uav)
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		textureDesc.MiscFlags = m_generate_mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		if (FAILED(device->CreateTexture1D(&textureDesc, nullptr, &m_texture1d)))
		{
			Game::Log << LOG_ERROR << ("Failed to create texture.");
			return;
		}

		if ((textureDesc.BindFlags &  D3D11_BIND_DEPTH_STENCIL) != 0)
		{
			// Create the depth/stencil view for the texture.
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			depthStencilViewDesc.Format = m_DepthStencilViewFormat;
			depthStencilViewDesc.Flags = 0;

			if (m_numslices > 1)
			{
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
				depthStencilViewDesc.Texture1DArray.MipSlice = 0;
				depthStencilViewDesc.Texture1DArray.FirstArraySlice = 0;
				depthStencilViewDesc.Texture1DArray.ArraySize = m_numslices;
			}
			else
			{
				depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
				depthStencilViewDesc.Texture1D.MipSlice = 0;
			}

			if (FAILED(device->CreateDepthStencilView(m_texture1d, &depthStencilViewDesc, &m_depthstencil_view)))
			{
				Game::Log << LOG_ERROR << ("Failed to create depth/stencil view.");
			}
		}

		if ((textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
		{
			// Create a Shader resource view for the texture.
			D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
			resourceViewDesc.Format = m_ShaderResourceViewFormat;

			if (m_numslices > 1)
			{
				resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1DARRAY;
				resourceViewDesc.Texture1DArray.MipLevels = m_generate_mipmaps ? -1 : 1;
				resourceViewDesc.Texture1DArray.MostDetailedMip = 0;
				resourceViewDesc.Texture1DArray.FirstArraySlice = 0;
				resourceViewDesc.Texture1DArray.ArraySize = m_numslices;
			}
			else
			{
				resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
				resourceViewDesc.Texture1D.MipLevels = m_generate_mipmaps ? -1 : 1;
				resourceViewDesc.Texture1D.MostDetailedMip = 0;
			}

			if (FAILED(device->CreateShaderResourceView(m_texture1d, &resourceViewDesc, &m_srv)))
			{
				Game::Log << LOG_ERROR << ("Failed to create shader resource view.");
			}
			else if (m_generate_mipmaps)
			{
				devicecontext->GenerateMips(m_srv);
			}
		}

		if ((textureDesc.BindFlags & D3D11_BIND_RENDER_TARGET) != 0)
		{
			// Create the render target view for the texture.
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = m_RenderTargetViewFormat;

			if (m_numslices > 1)
			{
				renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
				renderTargetViewDesc.Texture1DArray.MipSlice = 0;
				renderTargetViewDesc.Texture1DArray.FirstArraySlice = 0;
				renderTargetViewDesc.Texture1DArray.ArraySize = m_numslices;
			}
			else
			{
				renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
				renderTargetViewDesc.Texture1D.MipSlice = 0;
			}

			if (FAILED(device->CreateRenderTargetView(m_texture1d, &renderTargetViewDesc, &m_rendertarget_view)))
			{
				Game::Log << LOG_ERROR << ("Failed to create render target view.");
			}
		}
		if ((textureDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0)
		{
			// UAVs cannot be multi-sampled.
			assert(m_SampleDesc.Count == 1);

			// Create a Shader resource view for the texture.
			D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
			unorderedAccessViewDesc.Format = m_UnorderedAccessViewFormat;

			if (m_numslices > 1)
			{
				unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				unorderedAccessViewDesc.Texture1DArray.FirstArraySlice = 0;
				unorderedAccessViewDesc.Texture1DArray.ArraySize = m_numslices;
				unorderedAccessViewDesc.Texture1DArray.MipSlice = 0;
			}
			else
			{
				unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				unorderedAccessViewDesc.Texture2D.MipSlice = 0;
			}

			if (FAILED(device->CreateUnorderedAccessView(m_texture1d, &unorderedAccessViewDesc, &m_uav)))
			{
				Game::Log << LOG_ERROR << ("Failed to create unordered access view.");
			}
		}

		assert(m_bpp > 0 && m_bpp % 8 == 0);
		m_buffer.resize(width * (m_bpp / 8));
	}
}

void TextureDX11::Resize2D(uint16_t width, uint16_t height)
{
	auto device = Game::Engine->GetDevice();
	auto devicecontext = Game::Engine->GetDeviceContext();

	if (m_width != width || m_height != height)
	{
		// Release resource before resizing
		m_texture2d= NULL();
		m_rendertarget_view= NULL();
		m_depthstencil_view= NULL();
		m_srv= NULL();
		m_uav= NULL();

		m_width = max(width, (uint16_t)1);
		m_height = max(height, (uint16_t)1);

		// Create texture with the dimensions specified.
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };

		textureDesc.ArraySize = m_numslices;
		textureDesc.Format = m_TextureResourceFormat;
		textureDesc.SampleDesc = m_SampleDesc;

		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.MipLevels = 1;

		if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Read) != 0)
		{
			textureDesc.Usage = D3D11_USAGE_STAGING;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		}
		else if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Write) != 0)
		{
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.CPUAccessFlags = 0;
		}

		if (!m_is_uav && !m_isdynamic && (m_DepthStencilViewFormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) != 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		if (!m_isdynamic && (m_RenderTargetViewFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Read) == 0)
		{
			textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (m_is_uav && !m_isdynamic)
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		textureDesc.MiscFlags = m_generate_mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &m_texture2d)))
		{
			Game::Log << LOG_ERROR << ("Failed to create texture.");
			return;
		}

		if ((textureDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0)
		{
			// Create the depth/stencil view for the texture.
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			depthStencilViewDesc.Format = m_DepthStencilViewFormat;
			depthStencilViewDesc.Flags = 0;

			if (m_numslices > 1)
			{
				if (m_SampleDesc.Count > 1)
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
					depthStencilViewDesc.Texture2DMSArray.FirstArraySlice = 0;
					depthStencilViewDesc.Texture2DMSArray.ArraySize = m_numslices;
				}
				else
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					depthStencilViewDesc.Texture2DArray.MipSlice = 0;
					depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
					depthStencilViewDesc.Texture2DArray.ArraySize = m_numslices;
				}
			}
			else
			{
				if (m_SampleDesc.Count > 1)
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					depthStencilViewDesc.Texture2D.MipSlice = 0;
				}
			}

			if (FAILED(device->CreateDepthStencilView(m_texture2d, &depthStencilViewDesc, &m_depthstencil_view)))
			{
				Game::Log << LOG_ERROR << ("Failed to create depth/stencil view.");
			}
		}

		if ((textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
		{
			// Create a Shader resource view for the texture.
			D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
			resourceViewDesc.Format = m_ShaderResourceViewFormat;

			if (m_numslices > 1)
			{
				if (m_SampleDesc.Count > 1)
				{
					resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
					resourceViewDesc.Texture2DMSArray.FirstArraySlice = 0;
					resourceViewDesc.Texture2DMSArray.ArraySize = m_numslices;
				}
				else
				{
					resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					resourceViewDesc.Texture2DArray.FirstArraySlice = 0;
					resourceViewDesc.Texture2DArray.ArraySize = m_numslices;
					resourceViewDesc.Texture2DArray.MipLevels = m_generate_mipmaps ? -1 : 1;
					resourceViewDesc.Texture2DArray.MostDetailedMip = 0;
				}
			}
			else
			{
				if (m_SampleDesc.Count > 1)
				{
					resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
					resourceViewDesc.Texture2D.MipLevels = m_generate_mipmaps ? -1 : 1;
					resourceViewDesc.Texture2D.MostDetailedMip = 0;
				}
			}

			if (FAILED(device->CreateShaderResourceView(m_texture2d, &resourceViewDesc, &m_srv)))
			{
				Game::Log << LOG_ERROR << ("Failed to create texture resource view.");
			}
			else if (m_generate_mipmaps)
			{
				devicecontext->GenerateMips(m_srv);
			}
		}

		if ((textureDesc.BindFlags & D3D11_BIND_RENDER_TARGET) != 0)
		{
			// Create the render target view for the texture.
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = m_RenderTargetViewFormat;

			if (m_numslices > 1)
			{
				if (m_SampleDesc.Count > 1)
				{
					renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
					renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
					renderTargetViewDesc.Texture2DArray.ArraySize = m_numslices;

				}
				else
				{
					renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					renderTargetViewDesc.Texture2DArray.MipSlice = 0;
					renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
					renderTargetViewDesc.Texture2DArray.ArraySize = m_numslices;
				}
			}
			else
			{
				if (m_SampleDesc.Count > 1)
				{
					renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				}
				else
				{
					renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					renderTargetViewDesc.Texture2D.MipSlice = 0;
				}
			}

			if (FAILED(device->CreateRenderTargetView(m_texture2d, &renderTargetViewDesc, &m_rendertarget_view)))
			{
				Game::Log << LOG_ERROR << ("Failed to create render target view.");
			}
		}

		if ((textureDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0)
		{
			// UAVs cannot be multi sampled.
			assert(m_SampleDesc.Count == 1);

			// Create a Shader resource view for the texture.
			D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
			unorderedAccessViewDesc.Format = m_UnorderedAccessViewFormat;

			if (m_numslices > 1)
			{
				unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				unorderedAccessViewDesc.Texture2DArray.MipSlice = 0;
				unorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
				unorderedAccessViewDesc.Texture2DArray.ArraySize = m_numslices;
			}
			else
			{
				unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				unorderedAccessViewDesc.Texture2D.MipSlice = 0;
			}

			if (FAILED(device->CreateUnorderedAccessView(m_texture2d, &unorderedAccessViewDesc, &m_uav)))
			{
				Game::Log << LOG_ERROR << ("Failed to create unordered access view.");
			}
		}

		assert(m_bpp > 0 && m_bpp % 8 == 0);
		m_buffer.resize(width * height * (m_bpp / 8));
	}
}

void TextureDX11::Resize3D(uint16_t width, uint16_t height, uint16_t depth)
{
	// TODO
}

void TextureDX11::ResizeCube(uint16_t size)
{
	// TODO
}

void TextureDX11::Resize(uint16_t width, uint16_t height, uint16_t depth)
{
	switch (m_dimension)
	{
		case Dimension::Texture1D:
		case Dimension::Texture1DArray:
			Resize1D(width);
			break;
		case Dimension::Texture2D:
		case Dimension::Texture2DArray:
			Resize2D(width, height);
			break;
		case Dimension::Texture3D:
			Resize3D(width, height, depth);
			break;
		case Texture::Dimension::TextureCube:
			ResizeCube(width);
			break;
		default:
			Game::Log << LOG_ERROR << ("Unknown texture dimension.");
			break;
	}

	return;
}

void TextureDX11::Plot(XMFLOAT2 coord, const uint8_t* pixel, size_t size)
{
	assert(m_bpp > 0 && m_bpp % 8 == 0);
	assert(coord.x < m_width && coord.y < m_height && size == (m_bpp / 8));

	uint8_t bytesPerPixel = (m_bpp / 8);
	uint32_t stride = m_width * bytesPerPixel;
	uint32_t index = (coord.x * bytesPerPixel) + (coord.y * stride);

	for (unsigned int i = 0; i < size; ++i)
	{
		m_buffer[index + i] = *(pixel + i);
	}
}

void TextureDX11::FetchPixel(XMFLOAT2 coord, uint8_t*& pixel, size_t size)
{
	assert(m_bpp > 0 && m_bpp % 8 == 0);
	assert(coord.x < m_width && coord.y < m_height && size == (m_bpp / 8));

	uint8_t bytesPerPixel = (m_bpp / 8);
	uint32_t stride = m_width * bytesPerPixel;
	uint32_t index = (coord.x * bytesPerPixel) + (coord.y * stride);
	pixel = &m_buffer[index];
}

void TextureDX11::Copy(TextureDX11 * srcTexture)
{
	auto devicecontext = Game::Engine->GetDeviceContext();
	if (srcTexture && srcTexture != this)
	{
		if (m_dimension == srcTexture->m_dimension &&
			m_width == srcTexture->m_width &&
			m_height == srcTexture->m_height)
		{
			switch (m_dimension)
			{
			case Dimension::Texture1D:
			case Dimension::Texture1DArray:
				devicecontext->CopyResource(m_texture1d, srcTexture->m_texture1d);
				break;
			case Texture::Dimension::Texture2D:
			case Texture::Dimension::Texture2DArray:
				devicecontext->CopyResource(m_texture2d, srcTexture->m_texture2d);
				break;
			case Texture::Dimension::Texture3D:
			case Texture::Dimension::TextureCube:
				devicecontext->CopyResource(m_texture3d, srcTexture->m_texture3d);
				break;
			}
		}
		else
		{
			Game::Log << LOG_ERROR << ("Incompatible source texture.");
		}
	}

	if (((int)m_cpuaccess & (int)CPUGraphicsResourceAccess::Read) != 0 && m_texture2d)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// Copy the texture data from the texture resource
		if (FAILED(devicecontext->Map(m_texture2d, 0, D3D11_MAP_READ, 0, &mappedResource)))
		{
			Game::Log << LOG_ERROR << ("Failed to map texture resource for reading.");
		}

		memcpy_s(m_buffer.data(), m_buffer.size(), mappedResource.pData, m_buffer.size());

		devicecontext->Unmap(m_texture2d, 0);
	}
}

void TextureDX11::Clear(ClearFlags clearFlags, const FLOAT * float4_colour, float depth, uint8_t stencil)
{
	auto devicecontext = Game::Engine->GetDeviceContext();
	if (m_rendertarget_view && ((int)clearFlags & (int)ClearFlags::Colour) != 0)
	{
		devicecontext->ClearRenderTargetView(m_rendertarget_view, float4_colour);
	}

	{
		UINT flags = 0;
		flags |= ((int)clearFlags & (int)ClearFlags::Depth) != 0 ? D3D11_CLEAR_DEPTH : 0;
		flags |= ((int)clearFlags & (int)ClearFlags::Stencil) != 0 ? D3D11_CLEAR_STENCIL : 0;
		if (m_depthstencil_view && flags > 0)
		{
			devicecontext->ClearDepthStencilView(m_depthstencil_view, flags, depth, stencil);
		}
	}
}

void TextureDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype) const
{
	// NOTE: Removed previous logic to reload texture data when dirty.  Can reimplement if required

	auto devicecontext = Game::Engine->GetDeviceContext();

	if (parametertype == ShaderParameter::Type::Texture && m_srv)
	{
		ID3D11ShaderResourceView* srv[] = { m_srv };
		switch (shadertype)
		{
		case Shader::Type::VertexShader:
			devicecontext->VSSetShaderResources(slot_id, 1, srv);
			break;
		case Shader::Type::HullShader:
			devicecontext->HSSetShaderResources(slot_id, 1, srv);
			break;
		case Shader::Type::DomainShader:
			devicecontext->DSSetShaderResources(slot_id, 1, srv);
			break;
		case Shader::Type::GeometryShader:
			devicecontext->GSSetShaderResources(slot_id, 1, srv);
			break;
		case Shader::Type::PixelShader:
			devicecontext->PSSetShaderResources(slot_id, 1, srv);
			break;
		case Shader::Type::ComputeShader:
			devicecontext->CSSetShaderResources(slot_id, 1, srv);
			break;
		}
	}
	else if (parametertype == ShaderParameter::Type::RWTexture && m_uav)
	{
		ID3D11UnorderedAccessView* uav[] = { m_uav };
		switch (shadertype)
		{
		case Shader::Type::ComputeShader:
			devicecontext->CSSetUnorderedAccessViews(slot_id, 1, uav, nullptr);
			break;
		}
	}

}
void TextureDX11::Unbind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype) const
{
	auto devicecontext = Game::Engine->GetDeviceContext();

	if (parametertype == ShaderParameter::Type::Texture)
	{
		switch (shadertype)
		{
		case Shader::Type::VertexShader:
			devicecontext->VSSetShaderResources(slot_id, 1, null_srv);
			break;
		case Shader::Type::HullShader:
			devicecontext->HSSetShaderResources(slot_id, 1, null_srv);
			break;
		case Shader::Type::DomainShader:
			devicecontext->DSSetShaderResources(slot_id, 1, null_srv);
			break;
		case Shader::Type::GeometryShader:
			devicecontext->GSSetShaderResources(slot_id, 1, null_srv);
			break;
		case Shader::Type::PixelShader:
			devicecontext->PSSetShaderResources(slot_id, 1, null_srv);
			break;
		case Shader::Type::ComputeShader:
			devicecontext->CSSetShaderResources(slot_id, 1, null_srv);
			break;
		}
	}
	else if (parametertype == ShaderParameter::Type::RWTexture)
	{
		switch (shadertype)
		{
		case Shader::Type::ComputeShader:
			devicecontext->CSSetUnorderedAccessViews(slot_id, 1, null_uav, nullptr);
			break;
		}
	}
}

ID3D11Resource* TextureDX11::GetTextureResource() const
{
	ID3D11Resource* resource = nullptr;
	switch (m_dimension)
	{
	case Texture::Dimension::Texture1D:
	case Texture::Dimension::Texture1DArray:
		resource = m_texture1d;
		break;
	case Texture::Dimension::Texture2D:
	case Texture::Dimension::Texture2DArray:
		resource = m_texture2d;
		break;
	case Texture::Dimension::Texture3D:
	case Texture::Dimension::TextureCube:
		resource = m_texture3d;
		break;
	}

	return resource;
}

ID3D11ShaderResourceView* TextureDX11::GetShaderResourceView() const
{
	return m_srv;
}

ID3D11DepthStencilView* TextureDX11::GetDepthStencilView() const
{
	return m_depthstencil_view;
}

ID3D11RenderTargetView* TextureDX11::GetRenderTargetView() const
{
	return m_rendertarget_view;
}

ID3D11UnorderedAccessView* TextureDX11::GetUnorderedAccessView() const
{
	return m_uav;
}


// Global texture resources collection
TextureDX11 *TextureDX11::Get(const std::string & name)
{
	IndexedTextureCollection::const_iterator it = TextureResources.find(name);
	return (it != TextureResources.end() ? it->second : NULL);
}

// Global texture resources collection
bool TextureDX11::Exists(const std::string & name)
{
	return (TextureResources.find(name) != TextureResources.end());
}

// Global texture resources collection
bool TextureDX11::Store(const std::string & name, TextureDX11 *texture)
{
	if (name.empty() || texture == NULL) return false;
	if (Exists(name)) return false;

	TextureResources[name] = texture;
}

// Global texture resources collection
void TextureDX11::ShutdownGlobalTextureCollection(void)
{
	for (auto & item : TextureResources)
	{
		SafeDelete(item.second);
	}

	TextureResources.clear();
}

// Default destructor
TextureDX11::~TextureDX11(void)
{
	ReleaseIfExists(m_texture1d);
	ReleaseIfExists(m_texture2d);
	ReleaseIfExists(m_texture3d);
	ReleaseIfExists(m_srv);
	ReleaseIfExists(m_uav);
	ReleaseIfExists(m_rendertarget_view);
	ReleaseIfExists(m_depthstencil_view);
}









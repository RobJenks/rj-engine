#pragma once

#include <vector>
#include <unordered_map>
#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "ClearFlags.h"
#include "Shader.h"
#include "ShaderParameter.h"
#include "CPUGraphicsResourceAccess.h"
#include "Texture.h"


class TextureDX11 : public Texture
{
public:

	// Create an empty texture
	TextureDX11(void);

	// 1D Texture
	TextureDX11(uint16_t width, uint16_t slices, const TextureFormat & format, CPUGraphicsResourceAccess cpuAccess, bool bUAV = false);

	// 2D Texture
	TextureDX11(uint16_t width, uint16_t height, uint16_t slices, const TextureFormat & format, CPUGraphicsResourceAccess cpuAccess, bool bUAV = false);
	
	// 3D Texture
	enum Texture3DConstructor { Tex3d };	// Required to differentiate between 2D texture array and 3D textures
	TextureDX11(Texture3DConstructor, uint16_t width, uint16_t height, uint16_t depth, const TextureFormat & format, CPUGraphicsResourceAccess cpuAccess, bool bUAV = false);

	// Cube Texture
	enum CubeMapConstructor { Cube };		// Required to differentiate between 1D texture array and Cube textures
	TextureDX11(CubeMapConstructor, uint16_t size, uint16_t count, const TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool bUAV = false);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype) const;

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype) const;

	// Return the SRV that would be bound to a shader slot during Bind(), so that it can be group-bound during an external call for efficiency
	CMPINLINE const ID3D11ShaderResourceView * GetBindingSRV(void) const { return m_srv; }

	// Load a texture based on the supplied dimension type
	bool LoadTexture(const std::wstring & fileName, Texture::Dimension dimension);

	/**
	* Load a 2D texture from a file path.
	*/
	bool LoadTexture2D(const std::wstring& fileName);

	/**
	* Load a cubemap texture from a file path.
	*/
	bool LoadTextureCube(const std::wstring& fileName);

	/**
	* Generate mip maps for a texture.
	* For texture formats that don't support mipmapping,
	* this function does nothing.
	*/
	void GenerateMipMaps();

	/**
	* Get a pointer to a particular face of a cubemap texture.
	* For 1D, and 2D textures, this function always returns the only
	* face of the texture (the texture itself).
	*/
	TextureDX11 * GetFace(CubeFace face);

	/**
	* 3D textures store several slices of 2D textures.
	* Use this function to get a single 2D slice of a 3D texture.
	* For Cubemaps, this function can be used to get a face of the cubemap.
	* For 1D and 2D textures, this function will always return the texture
	* itself.
	*/
	TextureDX11 * GetSlice(unsigned int slice);

	// Get the width of the textures in texels.
	uint16_t GetWidth() const;
	// Get the height of the texture in texles.
	uint16_t GetHeight() const;
	// Get the depth of the texture in texels.
	uint16_t GetDepth() const;

	// Get the bits-per-pixel of the texture.
	uint8_t GetBPP() const;

	// Check to see if this texture has an alpha channel.
	bool IsTransparent() const;

	// Resize the texture to the new dimensions.
	// Resizing a texture will cause the original texture to be discarded.
	// Only use on "dynamic" textures (not ones loaded from a texture file).
	// @param width The width of the texture (for 1D, 2D, and 3D textures or size of a cubemap face for Cubemap textures)
	// @param height The height of the texture (for 2D, 3D textures)
	// @param depth The depth of the texture (for 3D textures only)
	void Resize(uint16_t width, uint16_t height = 0, uint16_t depth = 0);

	/**
	* Copy the contents of one texture into another.
	* Textures must both be the same size.
	*/
	void Copy(TextureDX11 *srcTexture);

	/**
	* Clear the texture.
	* @param color The color to clear the texture to.
	* @param depth The depth value to use for depth textures.
	* @param stencil The stencil value to use for depth/stencil textures.
	*/
	void Clear(ClearFlags clearFlags = ClearFlags::All, const FLOAT * float4_colour = float4_zero, float depth = 1.0f, uint8_t stencil = 0);

	// Gets the texture resource associated to this texture
	ID3D11Resource* GetTextureResource() const;

	// Gets the shader resource view for this texture so that it can be 
	// bound to a shader parameter.
	ID3D11ShaderResourceView* GetShaderResourceView() const;

	// Gets the depth stencil view if this is a depth/stencil texture.
	// Otherwise, this function will return null
	ID3D11DepthStencilView* GetDepthStencilView() const;

	// Get the render target view so the texture can be attached to a render target.
	ID3D11RenderTargetView* GetRenderTargetView() const;

	// Get the unordered access view so it can be bound to compute shaders and 
	// pixel shaders as a RWTexture
	ID3D11UnorderedAccessView* GetUnorderedAccessView() const;

	/**
	* Plot a color to the texture.
	* This method is only valid for texture created with CPUAccess::Write access.
	* @param coord The non-normalized texture coordinate.
	* @param color The color to plot (RGBA).
	*/
	template< typename T >
	void Plot(XMFLOAT2 coord, const T& color);

	/**
	* Retrieve the pixel at a particular location in the
	* texture.
	* This method is only valid for textures created with CPUAccess::Read access.
	* @param coord The non-normalized texture coordinate.
	* @return The pixel cast to the requested type.
	*/
	template< typename T >
	T FetchPixel(XMFLOAT2 coord);


	// Default destructor
	~TextureDX11(void);

private:

	void Plot(XMFLOAT2 coord, const uint8_t* pixel, size_t size);
	void FetchPixel(XMFLOAT2 coord, uint8_t*& pixel, size_t size);
	void Resize1D(uint16_t width);
	void Resize2D(uint16_t width, uint16_t height);
	void Resize3D(uint16_t width, uint16_t height, uint16_t depth);
	void ResizeCube(uint16_t size);
	

	// Instance fields
	ID3D11Texture1D *						m_texture1d;
	ID3D11Texture2D *						m_texture2d;
	ID3D11Texture3D *						m_texture3d;
	ID3D11ShaderResourceView *				m_srv;
	ID3D11UnorderedAccessView *				m_uav;
	ID3D11RenderTargetView *				m_rendertarget_view;
	ID3D11DepthStencilView *				m_depthstencil_view;

	TextureFormat							m_format;
	Dimension								m_dimension;
	uint16_t								m_width, m_height;
	uint16_t								m_numslices;			// For 3D and cube textures
	CPUGraphicsResourceAccess				m_cpuaccess;
	bool									m_isdynamic;			// If write access is supported
	bool									m_is_uav;

	// DXGI texture format support flags
	UINT									m_TextureResourceFormatSupport;
	UINT									m_DepthStencilViewFormatSupport;
	UINT									m_ShaderResourceViewFormatSupport;
	UINT									m_RenderTargetViewFormatSupport;
	UINT									m_UnorderedAccessViewFormatSupport;

	DXGI_FORMAT								m_TextureResourceFormat;
	DXGI_FORMAT								m_DepthStencilViewFormat;
	DXGI_FORMAT								m_RenderTargetViewFormat;
	DXGI_FORMAT								m_ShaderResourceViewFormat;
	DXGI_FORMAT								m_UnorderedAccessViewFormat;

	DXGI_SAMPLE_DESC						m_SampleDesc;
	bool									m_generate_mipmaps;
	uint8_t									m_bpp;				// bits-per-pixel
	uint16_t								m_pitch;			// bytes to next scanline
	bool									m_istransparent;

	// Buffer for dynamic texture data
	typedef uint8_t							ColourBufferData;
	typedef std::vector<ColourBufferData>	ColourBuffer;
	ColourBuffer							m_buffer;

	std::wstring							m_filename;


public:

	// Global collection of unique texture resources
	typedef std::unordered_map<std::string, TextureDX11*> IndexedTextureCollection;
	static IndexedTextureCollection TextureResources;
	static TextureDX11 *Get(const std::string & name);
	static bool Exists(const std::string & name);
	static bool Store(const std::string & name, TextureDX11 *texture);
	static void ShutdownGlobalTextureCollection(void);


private:

	// Working data
	static const FLOAT *						float4_zero;
	static ID3D11ShaderResourceView * const		null_srv[1];
	static ID3D11UnorderedAccessView * const	null_uav[1];

};




template< typename T >
void TextureDX11::Plot(XMFLOAT2 coord, const T& color)
{
	Plot(coord, (const uint8_t*)(&color), sizeof(T));
}

template< typename T >
T TextureDX11::FetchPixel(XMFLOAT2 coord)
{
	uint8_t* pixel = nullptr;
	FetchPixel(coord, pixel, sizeof(T));

	return *((T*)pixel);
}




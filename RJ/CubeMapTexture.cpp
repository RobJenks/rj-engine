#include "CubeMapTexture.h"

Result CubeMapTexture::Initialise(ID3D11Device *device, const char *filename)
{
	HRESULT result;

	// Set flags to signify that the data should be loaded into a cubemap texture
	D3DX11_IMAGE_LOAD_INFO imgloadinfo;
	imgloadinfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;	

	// Load the data into a texture object
	ID3D11Texture2D *SMTexture = 0;
	result = D3DX11CreateTextureFromFile(device, (LPCSTR)filename, &imgloadinfo, NULL, (ID3D11Resource**)&SMTexture, NULL);
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotLoadTextureData;
	}

	// Now create and populate a texture description object to get info on this loaded data
	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	// Prepare a shader resource description for loading the data
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	// Finally create the shader resource view using this data and description object
	result = device->CreateShaderResourceView(SMTexture, &SMViewDesc, &m_texture);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateShaderResourceViewFromTexture;
	}

	// Return success if the data was loaded successfully
	return ErrorCodes::NoError;
}

CubeMapTexture::CubeMapTexture(void)
{
}

CubeMapTexture::~CubeMapTexture(void)
{
}
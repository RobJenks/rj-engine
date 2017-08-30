#include <vector>
#include <fstream>
#include "ErrorCodes.h"
#include "Utility.h"
#include "DX11_Core.h"
#include "D3DCompiler.h"
#include "GameVarsExtern.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "Material.h"
#include "InputLayoutDesc.h"
#include "Data\\Shaders\\light_definition.h"
#include "Data\\Shaders\\standard_ps_const_buffer.h"

#include "ShaderManager.h"

// Loads a compiled shader object (*.cso) and returns the byte data
Result ShaderManager::LoadCompiledShader(const std::string & filename, byte **ppOutShader, SIZE_T *pOutBufferSize)
{
	// Attempt to open the file
	if (filename == NullString || !ppOutShader || !pOutBufferSize) return ErrorCodes::CannotLoadCompiledShaderWithNullInputData;
	std::ifstream sfile(filename, std::ios::in | std::ios::binary | std::ios::ate);

	// Test whether file could be successfully opened
	if (sfile.is_open())
	{
		// Determine the length of the file and allocate sufficient space
		int length = (int)sfile.tellg();
		if (length <= 0) return ErrorCodes::CompiledShaderAppearsToHoldNoData;

		// Store the data and attempt to allocate space for the shader
		(*pOutBufferSize) = (sizeof(byte) * length);
		(*ppOutShader) = new byte[length];
		if ((*ppOutShader) == NULL) return ErrorCodes::CouldNotAllocateSpaceForCompiledShader;

		// Retrieve the file data
		sfile.seekg(0, std::ios::beg);
		sfile.read(reinterpret_cast<char*>(&((*ppOutShader)[0])), length);
		sfile.close();
	}
	else
	{
		return ErrorCodes::CouldNotOpenCompiledShaderFile;
	}

	// Return success now that the file data was loaded
	return ErrorCodes::NoError; 
}

// Creates a new shader from the specified CSO
Result ShaderManager::CreateVertexShader(	ID3D11Device *device, const std::string & filename, InputLayoutDesc *layout_desc,
											ID3D11VertexShader **ppOutShader, ID3D11InputLayout **ppOutInputLayout)
{
	// Paramter check
	if (!device || !layout_desc) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	byte *shader = NULL; SIZE_T buffersize = 0;
	Result result = ShaderManager::LoadCompiledShader(filename, &shader, &buffersize);
	if (result != ErrorCodes::NoError) return result;
	if (shader == NULL || buffersize <= 0) return ErrorCodes::UnspecifiedErrorWhenLoadingCompiledShader;
	
	/*ID3DBlob *filedata = NULL;
	HRESULT hr = D3DReadFileToBlob((LPCWSTR)filename.c_str(), &filedata);
	HRESULT hr2 = device->CreateVertexShader(filedata->GetBufferPointer(), filedata->GetBufferSize(), NULL, ppOutShader);*/
	
	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreateVertexShader(shader, buffersize, NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateVertexShader;
	}

	// We now need to create the input layout, assuming we have been provided with valid data
	if (layout_desc->ElementCount() == 0U) return ErrorCodes::ShaderManagerCannotCreateInputLayoutWithoutDesc;
	hr = device->CreateInputLayout(layout_desc->Data(), layout_desc->ElementCount(), shader, buffersize, ppOutInputLayout);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateInputLayout;
	}

	// Deallocate the shader bytecode data
	if (shader) SafeDeleteArray(shader);

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

// Creates a new shader from the specified CSO
Result ShaderManager::CreatePixelShader(ID3D11Device *device, const std::string & filename, ID3D11PixelShader **ppOutShader)
{
	// Paramter check
	if (!device) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	byte *shader = NULL; SIZE_T buffersize = 0;
	Result result = ShaderManager::LoadCompiledShader(filename, &shader, &buffersize);
	if (result != ErrorCodes::NoError) return result;
	if (shader == NULL || buffersize <= 0) return ErrorCodes::UnspecifiedErrorWhenLoadingCompiledShader;

	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreatePixelShader(shader, buffersize, NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreatePixelShader;
	}

	// Deallocate the shader bytecode data
	if (shader) SafeDeleteArray(shader);

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

// Creates a new shader from the specified CSO
Result ShaderManager::CreateGeometryShader(ID3D11Device *device, const std::string & filename, ID3D11GeometryShader **ppOutShader)
{
	// Paramter check
	if (!device) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	byte *shader = NULL; SIZE_T buffersize = 0;
	Result result = ShaderManager::LoadCompiledShader(filename, &shader, &buffersize);
	if (result != ErrorCodes::NoError) return result;
	if (shader == NULL || buffersize <= 0) return ErrorCodes::UnspecifiedErrorWhenLoadingCompiledShader;

	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreateGeometryShader(shader, buffersize, NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateVertexShader;
	}

	// Deallocate the shader bytecode data
	if (shader) SafeDeleteArray(shader);

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

// Return a standard defined sampler description for use in shader initialisation
Result ShaderManager::GetStandardSamplerDescription(DefinedSamplerState type, D3D11_SAMPLER_DESC & outSamplerDesc)
{
	switch (type)
	{
		case DefinedSamplerState::StandardLinearSampler:
			outSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			outSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			outSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			outSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			outSamplerDesc.MipLODBias = 0.0f;
			outSamplerDesc.MaxAnisotropy = 1;
			outSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			outSamplerDesc.BorderColor[0] = 0;
			outSamplerDesc.BorderColor[1] = 0;
			outSamplerDesc.BorderColor[2] = 0;
			outSamplerDesc.BorderColor[3] = 0;
			outSamplerDesc.MinLOD = 0;
			outSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			return ErrorCodes::NoError;

		case DefinedSamplerState::StandardLinearClampSampler:
			outSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			outSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			outSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			outSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			outSamplerDesc.MipLODBias = 0.0f;
			outSamplerDesc.MaxAnisotropy = 1;
			outSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			outSamplerDesc.BorderColor[0] = 0;
			outSamplerDesc.BorderColor[1] = 0;
			outSamplerDesc.BorderColor[2] = 0;
			outSamplerDesc.BorderColor[3] = 0;
			outSamplerDesc.MinLOD = 0;
			outSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			return ErrorCodes::NoError;

		default:
			return ErrorCodes::ShaderManagerReceivedUnsupportedSamplerState;
	}
}

// Return a standard defined sampler state for use in shader initialisation
Result ShaderManager::CreateStandardSamplerState(DefinedSamplerState type, ID3D11Device *device, ID3D11SamplerState **ppOutSamplerState)
{
	// Parameter check
	if (!device || !ppOutSamplerState) return ErrorCodes::ShaderManagerCannotCreateSamplerWithNullInput;

	// We will create a new sampler description
	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	// Attempt to retrieve the relevant sampler state description
	Result result = GetStandardSamplerDescription(type, desc);
	if (result != ErrorCodes::NoError) return result;

	// Attempt to create the sampler state based on this description
	HRESULT hr = device->CreateSamplerState(&desc, ppOutSamplerState);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerFailedToCreateSamplerState;
	}

	// We have created the sampler state so return success
	return ErrorCodes::NoError;
}


// Create a standard dynamic constant buffer of the specified size (Usage=Dynamic, BindFlags=ConstantBuffer, CPUAccessFlags=Write, 
// MiscFlags = 0, StructureByteStride = 0, ByteWidth = @bytewidth)
Result ShaderManager::CreateStandardDynamicConstantBuffer(UINT bytewidth, ID3D11Device *device, ID3D11Buffer **ppOutConstantBuffer)
{
	return CreateBuffer(D3D11_USAGE_DYNAMIC, bytewidth, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0, device, ppOutConstantBuffer);
}

// Create a buffer based on the specified parameters
Result ShaderManager::CreateBuffer(	D3D11_USAGE usage, UINT bytewidth, UINT bindflags, UINT cpuaccessflags, UINT miscflags, UINT structurebytestride, 
									ID3D11Device *device, ID3D11Buffer **ppOutBuffer)
{
	// Parameter check
	if (!device || !ppOutBuffer) return ErrorCodes::ShaderManagerCannotCreateBufferWithNullInput;
	if (bytewidth <= 0U) return ErrorCodes::ShaderManagerCannotAllocateNullBuffer;
	if (bytewidth > ShaderManager::MAX_BUFFER_ALLOCATION) return ErrorCodes::ShaderManagerCannotExceedBufferAllocationLimit;

	// Create a new buffer description based on the supplied data
	D3D11_BUFFER_DESC desc;
	desc.Usage = usage;
	desc.ByteWidth = bytewidth;
	desc.BindFlags = bindflags;
	desc.CPUAccessFlags = cpuaccessflags;
	desc.MiscFlags = miscflags;
	desc.StructureByteStride = structurebytestride;

	// Attempt to create a new buffer based on this description
	HRESULT hr = device->CreateBuffer(&desc, NULL, ppOutBuffer);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateBuffer;
	}

	// We have created the buffer so return success
	return ErrorCodes::NoError;
}


// Populate one of the standard constant buffer objects with appropriate data
Result ShaderManager::PopulateConstantBuffer(StandardPSConstBuffer *buffer)
{
	// Parameter check
	if (!buffer) return ErrorCodes::CannotPopulateNullConstantBuffer;

	// Populate each field in turn
	buffer->EyeWorldPos = Game::Engine->GetCamera()->GetPositionF();

	// Populate lighting data
	unsigned int light_count = (unsigned int)Game::Engine->LightingManager.GetLightSourceCount(); 
	buffer->LightCount = light_count; 

	// Copy all lighting data into the buffer
	memcpy(buffer->Lights, Game::Engine->LightingManager.GetLightData(), sizeof(LightData) * light_count);

	// Copy all material data into the buffer
	if (Game::Engine->GetCurrentModelBuffer())
	{
		unsigned int material_count = Game::Engine->GetCurrentModelBuffer()->GetMaterialCount();
		buffer->MaterialCount = material_count;
		memcpy(buffer->Materials, Game::Engine->GetCurrentModelBuffer()->GetMaterialData(), sizeof(Material) * material_count);
	}

	// Return success
	return ErrorCodes::NoError;
}

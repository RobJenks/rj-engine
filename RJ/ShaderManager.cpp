#include <vector>
#include <fstream>
#include "ErrorCodes.h"
#include "Utility.h"
#include "DX11_Core.h"
#include "InputLayoutDesc.h"

#include "ShaderManager.h"

// Loads a compiled shader object (*.cso) and returns the byte data
Result ShaderManager::LoadCompiledShader(const std::string & filename, std::vector<char> outShader)
{
	// Attempt to open the file
	if (filename == NullString) return ErrorCodes::CannotLoadCompiledShaderWithNullInputFile;
	std::ifstream sfile(filename, std::ios::in | std::ios::binary | std::ios::ate);

	// if open was successful
	if (sfile.is_open())
	{
		// Determine the length of the file and clear/reserve sufficient space
		outShader.clear(); 
		std::basic_istream<char>::pos_type length = sfile.tellg();
		outShader.reserve((std::vector<char>::size_type)length);

		// Retrieve the file data
		sfile.seekg(0, std::ios::beg);
		sfile.read(reinterpret_cast<char*>(&outShader[0]), length);
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
Result CreateVertexShader(	ID3D11Device *device, const std::string & filename, InputLayoutDesc *layout_desc,
							ID3D11VertexShader **ppOutShader, ID3D11InputLayout **ppOutInputLayout)
{
	// Paramter check
	if (!device || !layout_desc) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	std::vector<char> shader;
	Result result = ShaderManager::LoadCompiledShader(filename, shader);
	if (result != ErrorCodes::NoError) return result;

	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreateVertexShader(&(shader[0]), (SIZE_T)shader.size(), NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateVertexShader;
	}

	// We now need to create the input layout, assuming we have been provided with valid data
	if (layout_desc->ElementCount() == 0U) return ErrorCodes::ShaderManagerCannotCreateInputLayoutWithoutDesc;
	hr = device->CreateInputLayout(layout_desc->Data(), layout_desc->ElementCount(), &(shader[0]), (SIZE_T)shader.size(), ppOutInputLayout);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateInputLayout;
	}

	// Clear the shader bytecode data (although it should be deallocated when the method ends)
	shader.clear();

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

// Creates a new shader from the specified CSO
Result ShaderManager::CreatePixelShader(ID3D11Device *device, const std::string & filename, ID3D11PixelShader **ppOutShader)
{
	// Paramter check
	if (!device) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	std::vector<char> shader;
	Result result = ShaderManager::LoadCompiledShader(filename, shader);
	if (result != ErrorCodes::NoError) return result;

	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreatePixelShader(&(shader[0]), (SIZE_T)shader.size(), NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreatePixelShader;
	}

	// Clear the shader bytecode data (although it should be deallocated when the method ends)
	shader.clear();

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

// Creates a new shader from the specified CSO
Result ShaderManager::CreateGeometryShader(ID3D11Device *device, const std::string & filename, ID3D11GeometryShader **ppOutShader)
{
	// Paramter check
	if (!device) return ErrorCodes::ShaderManagerCannotCreateShaderWithNullInput;

	// Attempt to load the data from external compiled shader object (cso) file
	std::vector<char> shader;
	Result result = ShaderManager::LoadCompiledShader(filename, shader);
	if (result != ErrorCodes::NoError) return result;

	// Now attempt to create the shader based on this data
	HRESULT hr = device->CreateGeometryShader(&(shader[0]), (SIZE_T)shader.size(), NULL, ppOutShader);
	if (FAILED(hr))
	{
		return ErrorCodes::ShaderManagerCouldNotCreateVertexShader;
	}

	// Clear the shader bytecode data (although it should be deallocated when the method ends)
	shader.clear();

	// The shader was created successfully, so return success here
	return ErrorCodes::NoError;
}

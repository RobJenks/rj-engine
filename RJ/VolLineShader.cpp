#include "DX11_Core.h"
#include "DXLocaliser.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"

#include "VolLineShader.h"

VolLineShader::VolLineShader(const DXLocaliser *locale)
{
	// Store a reference to the current locale
	m_locale = locale;

	// Set pointers to NULL
	m_vertexShader = NULL;
	m_pixelShader = NULL;
	m_inputlayout = NULL;
	m_sampleState = NULL;
	m_cbuffer_vs = NULL;
	m_cbuffer_gs = NULL;
	m_cbuffer_ps = NULL;
}


Result VolLineShader::Initialise(ID3D11Device *device)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, ShaderFilename("vol_line.vs.cso"));
	if (result != ErrorCodes::NoError) return result;
	
	result = InitialiseGeometryShader(device, ShaderFilename("vol_line.gs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, ShaderFilename("vol_line.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}

// Initialise shader
Result VolLineShader::InitialiseVertexShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::CannotInitialiseVolLineShaderVSWithNullInput;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineVertexShader;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType), device, &m_cbuffer_vs);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineVSConstantBuffer;

	// Return success
	return ErrorCodes::NoError;
}

// Initialise shader
Result VolLineShader::InitialiseGeometryShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::CannotInitialiseVolLineShaderGSWithNullInput;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateGeometryShader(device, filename, &m_geometryShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineGeometryShader;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(GSBufferType), device, &m_cbuffer_gs);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineGSConstantBuffer;

	// Return success
	return ErrorCodes::NoError;
}

// Initialise shader
Result VolLineShader::InitialisePixelShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::CannotInitialiseVolLineShaderPSWithNullInput;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLinePixelShader;

	// Use a standard linear sampler
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearSampler, device, &m_sampleState);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLinePSSamplerState;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(PSBufferType), device, &m_cbuffer_ps);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLinePSConstantBuffer;

	// Return success
	return ErrorCodes::NoError;
}



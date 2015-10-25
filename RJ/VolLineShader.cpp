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
	if (!device || filename == NullString) return ErrorCodes::CannotInitialiseVolLineShaderWithNullInput;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineVertexShader;

	// Return success
	return ErrorCodes::NoError;
}






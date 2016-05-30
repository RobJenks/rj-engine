////////////////////////////////////////////////////////////////////////////////
// Filename: LightFadeShader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "Data\\Shaders\\light_definition.h"
#include "Data\\Shaders\\standard_ps_const_buffer.h"

#include "LightFadeShader.h"


LightFadeShader::LightFadeShader(void)
{
	// Set pointers to NULL
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_inputlayout = 0;
	m_sampleState = 0;
	m_cbuffer_vs = 0;
	m_cbuffer_ps = 0;
}


Result LightFadeShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, ShaderFilename("light_fade.vs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, ShaderFilename("light_fade.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result LightFadeShader::InitialiseVertexShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateLightFadeShaderVS;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc;
	bool layout_loaded = InputLayoutDesc::GetStandardLayout("Vertex_Inst_TexNormMatLit", layout_desc);
	if (!layout_loaded) return ErrorCodes::CouldNotRetrieveShaderInputLayout;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightFadeShaderVS;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType), device, &m_cbuffer_vs);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightFadeShaderConstBuffersVS;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result LightFadeShader::InitialisePixelShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateLightFadeShaderPS;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightFadeShaderPS;

	// Use a standard linear sampler
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearSampler, device, &m_sampleState);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightFadeShaderSamplerPS;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(StandardPSConstBuffer), device, &m_cbuffer_ps);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightFadeShaderConstBuffersPS;

	// Return success
	return ErrorCodes::NoError;
}

Result XM_CALLCONV LightFadeShader::Render(ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
	const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType *vsbuffer; StandardPSConstBuffer *psbuffer;

	// Parameter check
	if (!deviceContext || vertexCount <= 0 || indexCount <= 0 || instanceCount <= 0 || !texture) return ErrorCodes::InvalidShaderParameters;

	// Initialise vertex shader constant buffer
	hr = deviceContext->Map(m_cbuffer_vs, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	vsbuffer = (VSBufferType*)mappedResource.pData;
	{
		XMStoreFloat4x4(&vsbuffer->view, XMMatrixTranspose(viewMatrix));				// Transpose matrix
		XMStoreFloat4x4(&vsbuffer->projection, XMMatrixTranspose(projectionMatrix));	// Transpose matrix
	}
	deviceContext->Unmap(m_cbuffer_vs, 0);
	deviceContext->VSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_vs);

	// Initialise pixel shader constant buffer
	hr = deviceContext->Map(m_cbuffer_ps, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	psbuffer = (StandardPSConstBuffer*)mappedResource.pData;
	{
		Game::Engine->ShaderManager.PopulateConstantBuffer(psbuffer);
	}
	deviceContext->Unmap(m_cbuffer_ps, 0);
	deviceContext->PSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_ps);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_inputlayout);

	// Activate the shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the model
	deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);

	// Return success
	return ErrorCodes::NoError;
}

void LightFadeShader::Shutdown()
{
	// Release all resources
	ReleaseIfExists(m_cbuffer_vs);
	ReleaseIfExists(m_cbuffer_ps);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_sampleState);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_pixelShader);
}


LightFadeShader::~LightFadeShader()
{
}

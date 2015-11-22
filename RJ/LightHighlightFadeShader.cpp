////////////////////////////////////////////////////////////////////////////////
// Filename: LightHighlightFadeShader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"

#include "LightHighlightFadeShader.h"


LightHighlightFadeShader::LightHighlightFadeShader(void)
{
	// Set pointers to NULL
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_inputlayout = 0;
	m_sampleState = 0;
	m_cbuffer_vs = 0;
	m_cbuffer_ps = 0;
}


Result LightHighlightFadeShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, ShaderFilename("light_highlight_fade.vs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, ShaderFilename("light_highlight_fade.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result LightHighlightFadeShader::InitialiseVertexShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateLightHFShaderVS;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("mTransform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("iParams", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightHFShaderVS;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType), device, &m_cbuffer_vs);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightHFShaderConstBuffersVS;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result LightHighlightFadeShader::InitialisePixelShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateLightHFShaderPS;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightHFShaderPS;

	// Use a standard linear sampler
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearSampler, device, &m_sampleState);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightHFShaderSamplerPS;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(PSBufferType), device, &m_cbuffer_ps);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingLightHFShaderConstBuffersPS;

	// Return success
	return ErrorCodes::NoError;
}

Result XM_CALLCONV LightHighlightFadeShader::Render(ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
	const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType *vsbuffer; PSBufferType *psbuffer;

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
	psbuffer = (PSBufferType*)mappedResource.pData;
	{
		psbuffer->lightDirection = m_lightdirection;
		psbuffer->ambientColor = m_ambientcolour;
		psbuffer->diffuseColor = m_diffusecolour;
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

// Sets the parameters specific to the light shader, i.e. light type / direction / colour
Result LightHighlightFadeShader::SetLightParameters(XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor)
{
	// Store the new light/alpha parameters; these will take effect in the next call to SetShaderParameters (each frame)
	m_lightdirection = lightDirection;
	m_ambientcolour = ambientColor;
	m_diffusecolour = diffuseColor;

	// Return success
	return ErrorCodes::NoError;
}


void LightHighlightFadeShader::Shutdown()
{
	// Release all resources
	ReleaseIfExists(m_cbuffer_vs);
	ReleaseIfExists(m_cbuffer_ps);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_sampleState);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_pixelShader);
}


LightHighlightFadeShader::~LightHighlightFadeShader()
{
}

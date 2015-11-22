#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "iShader.h"

#include "FireShader.h"


FireShader::FireShader(void)
{
	// Set pointers to null before we attempt initialisation
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_inputlayout = 0;
	m_cbuffer_vs1 = 0;
	m_cbuffer_vs2 = 0;
	m_cbuffer_ps = 0;
	m_sampleState1 = 0;
	m_sampleState2 = 0;
}

Result FireShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, iShader::ShaderFilename("fire.vs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, iShader::ShaderFilename("fire.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result FireShader::InitialiseVertexShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateFireShaderVS;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderVS;

	// Create the first shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType1), device, &m_cbuffer_vs1);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderConstBuffersVS;

	// Create the second shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType2), device, &m_cbuffer_vs2);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderConstBuffersVS;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result FireShader::InitialisePixelShader(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateFireShaderPS;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderPS;

	// This shader uses two samplers; first, a standard linear wrap sampler
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearSampler, device, &m_sampleState1);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderSamplerPS;

	// Second, a linear clamp sampler
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearClampSampler, device, &m_sampleState2);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingFireShaderSamplerPS;

	// Return success
	return ErrorCodes::NoError;
}


void FireShader::Shutdown(void)
{
	// Release all resources
	ReleaseIfExists(m_cbuffer_vs1);
	ReleaseIfExists(m_cbuffer_vs2);
	ReleaseIfExists(m_cbuffer_ps);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_sampleState1);
	ReleaseIfExists(m_sampleState2);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_pixelShader);
}


Result XM_CALLCONV FireShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, const FXMMATRIX worldMatrix, const CXMMATRIX viewMatrix,
							 const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
							 ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
							 XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2,
							 XMFLOAT2 distortion3, float distortionScale, float distortionBias)
{

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType1 *vsbuffer1; VSBufferType2 *vsbuffer2; PSBufferType *psbuffer;

	// Parameter check
	if (!deviceContext || indexCount <= 0 || !fireTexture || !noiseTexture || !alphaTexture) return ErrorCodes::InvalidShaderParameters;

	// Initialise the first vertex shader constant buffer
	hr = deviceContext->Map(m_cbuffer_vs1, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	vsbuffer1 = (VSBufferType1*)mappedResource.pData;
	{
		XMStoreFloat4x4(&vsbuffer1->view, XMMatrixTranspose(viewMatrix));				// Transpose matrix
		XMStoreFloat4x4(&vsbuffer1->projection, XMMatrixTranspose(projectionMatrix));	// Transpose matrix
		XMStoreFloat4x4(&vsbuffer1->world, XMMatrixTranspose(worldMatrix));				// Transpose matrix
	}
	deviceContext->Unmap(m_cbuffer_vs1, 0);
	deviceContext->VSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_vs1);

	// Initialise the second vertex shader constant buffer
	hr = deviceContext->Map(m_cbuffer_vs2, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	vsbuffer2 = (VSBufferType2*)mappedResource.pData;
	{
		vsbuffer2->frameTime = frameTime;
		vsbuffer2->scrollSpeeds = scrollSpeeds;
		vsbuffer2->scales = scales;
	}
	deviceContext->Unmap(m_cbuffer_vs2, 0);
	deviceContext->VSSetConstantBuffers((unsigned int)1U, 1, &m_cbuffer_vs2);

	// Initialise pixel shader constant buffer
	hr = deviceContext->Map(m_cbuffer_ps, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	psbuffer = (PSBufferType*)mappedResource.pData;
	{
		psbuffer->distortion1 = distortion1;
		psbuffer->distortion2 = distortion2;
		psbuffer->distortion3 = distortion3;
		psbuffer->distortionBias = distortionBias;
		psbuffer->distortionScale = distortionScale;
	}
	deviceContext->Unmap(m_cbuffer_ps, 0);
	deviceContext->PSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_ps);

	// Set the three shader texture resources in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &fireTexture);
	deviceContext->PSSetShaderResources(1, 1, &noiseTexture);
	deviceContext->PSSetShaderResources(2, 1, &alphaTexture);

	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_inputlayout);

	// Activate the shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &m_sampleState1);
	deviceContext->PSSetSamplers(1, 1, &m_sampleState2);

	// Render the model
	deviceContext->DrawIndexed(indexCount, 0, 0);

	// Return success
	return ErrorCodes::NoError;


}

FireShader::~FireShader(void) 
{ 

}

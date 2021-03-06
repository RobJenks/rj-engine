#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "iShader.h"

#include "TexcubeShader.h"


TexcubeShader::TexcubeShader(void)
{
	// Set pointers to null before we attempt initialisation
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_inputlayout = 0;
	m_cbuffer_vs = 0;
	m_sampleState = 0;
}

Result TexcubeShader::Initialise(Rendering::RenderDeviceType * device, HWND hwnd)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, iShader::ShaderFilename("texcube.vs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, iShader::ShaderFilename("texcube.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result TexcubeShader::InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateTexcubeShaderVS;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingTexcubeShaderVS;

	// Create the shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(VSBufferType), device, &m_cbuffer_vs);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingTexcubeShaderConstBuffersVS;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result TexcubeShader::InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateTexcubeShaderPS;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingTexcubeShaderPS;

	// Use a standard linear sampler with clamp (rather than wrap) for all tex coords
	result = ShaderManager::CreateStandardSamplerState(ShaderManager::DefinedSamplerState::StandardLinearClampSampler, device, &m_sampleState);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingTexcubeShaderSamplerPS;

	// Return success
	return ErrorCodes::NoError;
}


void TexcubeShader::Shutdown(void)
{
	// Release all resources
	ReleaseIfExists(m_cbuffer_vs);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_sampleState);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_pixelShader);
}


Result RJ_XM_CALLCONV TexcubeShader::Render(Rendering::RenderDeviceContextType * deviceContext, int indexCount, const FXMMATRIX worldMatrix,
								const CXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType *vsbuffer;

	// Parameter check
	if (!deviceContext || indexCount <= 0 || !texture) return ErrorCodes::InvalidShaderParameters;

	// Initialise vertex shader constant buffer
	hr = deviceContext->Map(m_cbuffer_vs, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	vsbuffer = (VSBufferType*)mappedResource.pData;
	{
		XMStoreFloat4x4(&vsbuffer->view, XMMatrixTranspose(viewMatrix));				// Transpose matrix
		XMStoreFloat4x4(&vsbuffer->projection, XMMatrixTranspose(projectionMatrix));	// Transpose matrix
		XMStoreFloat4x4(&vsbuffer->world, XMMatrixTranspose(worldMatrix));				// Transpose matrix
	}
	deviceContext->Unmap(m_cbuffer_vs, 0);
	deviceContext->VSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_vs);

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
	deviceContext->DrawIndexed(indexCount, 0, 0);

	// Return success
	return ErrorCodes::NoError;
}


TexcubeShader::~TexcubeShader(void) 
{ 

}

#include "DX11_Core.h"
#include "Utility.h"
#include "DXLocaliser.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "ModelBuffer.h"

#include "VolLineShader.h"

// Static two-vertex base model used as input to the geometry shader, which will then break out into a volumetric line
ModelBuffer *VolLineShader::BaseModel = NULL;

// Constructor
VolLineShader::VolLineShader(const DXLocaliser *locale)
{
	// Store a reference to the current locale
	m_locale = locale;

	// Set pointers to NULL
	m_vertexShader = NULL;
	m_geometryShader = NULL;
	m_pixelShader = NULL;
	m_inputlayout = NULL;
	m_sampleState = NULL;
	m_cbuffer_vs = NULL;
	m_cbuffer_gs = NULL;
	m_cbuffer_ps = NULL;

	// Initialise all shader parameters to default values
	m_viewport_size = XMFLOAT2(1024.0f, 768.0f); 
	m_clip_near = 1.0f;
	m_clip_far = 100.0f;
	m_radius = 4.0f;
}


Result VolLineShader::Initialise(ID3D11Device *device, XMFLOAT2 viewport_size, float clip_near, float clip_far)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, ShaderFilename((true ? "vol_line.vs.cso" : "tmp_shader.hlsl")));
	if (result != ErrorCodes::NoError) return result;
	
	result = InitialiseGeometryShader(device, ShaderFilename("vol_line.gs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, ShaderFilename("vol_line.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Store other static data that is used by the shader
	m_viewport_size = viewport_size;
	m_clip_near = clip_near;
	m_clip_far = clip_far;

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
		.Add("POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("mTransform",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,								D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform",	2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform",	3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("iParams",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1);
		
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

// Renders the shader.
Result XM_CALLCONV VolLineShader::Render(ID3D11DeviceContext *deviceContext, unsigned int vertexCount, unsigned int indexCount, unsigned int instanceCount,
								const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType *vsbuffer; GSBufferType *gsbuffer; PSBufferType *psbuffer;

	// Parameter check
	if (!deviceContext || vertexCount <= 0 || indexCount <= 0 || instanceCount <= 0 || !texture) return ErrorCodes::InvalidShaderParameters;

	// Initialise vertex shader constant buffer
	hr = deviceContext->Map(m_cbuffer_vs, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	vsbuffer = (VSBufferType*)mappedResource.pData;
	{
		XMStoreFloat4x4(&vsbuffer->viewmatrix, XMMatrixTranspose(viewMatrix));				// Transpose matrix
	}
	deviceContext->Unmap(m_cbuffer_vs, 0);
	deviceContext->VSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_vs);

	// Initialise geometry shader constant buffer
	hr = deviceContext->Map(m_cbuffer_gs, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	gsbuffer = (GSBufferType*)mappedResource.pData;
	{
		XMStoreFloat4x4(&gsbuffer->projectionmatrix, XMMatrixTranspose(projectionMatrix));	// Transpose matrix
		gsbuffer->radius = m_radius;
	}
	deviceContext->Unmap(m_cbuffer_gs, 0);
	deviceContext->GSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_gs);

	// Initialise pixel shader constant buffer
	hr = deviceContext->Map(m_cbuffer_ps, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	psbuffer = (PSBufferType*)mappedResource.pData;
	{
		psbuffer->radius = m_radius;
		psbuffer->viewport_size = m_viewport_size;
		psbuffer->clipdistance_front = m_clip_near;
		psbuffer->clipdistance_far = m_clip_far;
	}
	deviceContext->Unmap(m_cbuffer_ps, 0);
	deviceContext->PSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_ps);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_inputlayout);

	// Activate the shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the model
	deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);

	// Return success
	return ErrorCodes::NoError;
}


// Shut down and deallocate all resources
void VolLineShader::Shutdown()
{
	// Release all resources
	ReleaseIfExists(m_cbuffer_vs);
	ReleaseIfExists(m_cbuffer_gs);
	ReleaseIfExists(m_cbuffer_ps);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_sampleState);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_geometryShader);
	ReleaseIfExists(m_pixelShader);
}

// Initialise the static data used in volumetric line rendering
Result VolLineShader::InitialiseStaticData(ID3D11Device *device)
{
	// Create a new static base model
	VolLineShader::BaseModel = NULL;
	VolLineShader::BaseModel = new ModelBuffer();
	if (!VolLineShader::BaseModel) return ErrorCodes::CannotInitialiseStaticVolLineShaderData;

	// Create the base vertex and index data
	XMFLOAT4 *v = new XMFLOAT4[2] { XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	UINT16 *i = new UINT16[2] { 0U, 1U };

	// Initialise the base model using this data
	Result result = VolLineShader::BaseModel->Initialise(device, (const void**)&v, sizeof(XMFLOAT4), 2U, (const void**)&i, sizeof(UINT16), 2U);
	if (result != ErrorCodes::NoError) return result;

	// Deallocate the model vertex and index data
	SafeDelete(v); 
	SafeDelete(i);

	// Return success
	return ErrorCodes::NoError;
}










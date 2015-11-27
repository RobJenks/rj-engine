#include "DX11_Core.h"
#include "Utility.h"

#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "ModelBuffer.h"
#include "CoreEngine.h"
#include "Texture.h"
#include "VolumetricLine.h"

#include "VolLineShader.h"

// Static resources used during line rendering
ModelBuffer *										VolLineShader::BaseModel = NULL;
std::unordered_map<Texture*, ModelBuffer*>			VolLineShader::LineModels;
Texture *											VolLineShader::LinearDepthTextureObject = NULL;
ID3D11ShaderResourceView *							VolLineShader::LinearDepthTexture = NULL;
ID3D11ShaderResourceView **							VolLineShader::PSShaderResources = NULL;

// Constructor
VolLineShader::VolLineShader(void)
{
	// Set pointers to NULL
	m_vertexShader = NULL;
	m_geometryShader = NULL;
	m_pixelShader = NULL;
	m_pixelShader_tex = NULL;
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
	result = InitialiseVertexShader(device, ShaderFilename("vol_line.vs.cso"));
	if (result != ErrorCodes::NoError) return result;
	
	result = InitialiseGeometryShader(device, ShaderFilename("vol_line.gs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, ShaderFilename("vol_line.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShaderTextured(device, ShaderFilename("vol_line_tex.ps.cso"));
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
		.Add("POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,									D3D11_INPUT_PER_VERTEX_DATA, 0)
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

// Initialise shader
Result VolLineShader::InitialisePixelShaderTextured(ID3D11Device *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::CannotInitialiseVolLineShaderTexPSWithNullInput;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader_tex);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingVolLineTexPixelShader;

	// We do not need to create other resources, e.g. sampler states or constant buffers, since we can share with the base pixel shader

	// Return success
	return ErrorCodes::NoError;
}

// Renders the shader.
Result XM_CALLCONV VolLineShader::Render(	ID3D11DeviceContext *deviceContext, unsigned int vertexCount, unsigned int indexCount, unsigned int instanceCount,
											const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VSBufferType *vsbuffer; GSBufferType *gsbuffer; PSBufferType *psbuffer;

	// Parameter check
	if (!deviceContext || vertexCount <= 0 || indexCount <= 0 || instanceCount <= 0) return ErrorCodes::InvalidShaderParameters;

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
	}
	deviceContext->Unmap(m_cbuffer_gs, 0);
	deviceContext->GSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_gs);

	// Initialise pixel shader constant buffer
	hr = deviceContext->Map(m_cbuffer_ps, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) return ErrorCodes::CouldNotObtainShaderBufferLock;
	psbuffer = (PSBufferType*)mappedResource.pData;
	{
		psbuffer->clipdistance_front = m_clip_near;
		psbuffer->clipdistance_far = m_clip_far;
		psbuffer->viewport_size = m_viewport_size;
	}
	deviceContext->Unmap(m_cbuffer_ps, 0);
	deviceContext->PSSetConstantBuffers((unsigned int)0U, 1, &m_cbuffer_ps);

	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_inputlayout);

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Activate the vertex and geometry shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);

	// This shader will select from different pixel shaders depending on the type of rendering required
	if (texture)
	{
		// Render texture is required; use the textured rendering shader and assign additional shader resoures as required
		VolLineShader::PSShaderResources[1] = texture;
		deviceContext->PSSetShaderResources(0, 2, VolLineShader::PSShaderResources);
		deviceContext->PSSetShader(m_pixelShader_tex, NULL, 0);
	}
	else
	{
		// No render texture required; we need only the standard linear depth texture and regular pixel shader
		deviceContext->PSSetShaderResources(0, 1, &VolLineShader::LinearDepthTexture);
		deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	}

	// Render the model
	deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);

	// Deactivate any non-standard pipeline shaders (anything except the mandatory vertex and pixel shader)
	deviceContext->GSSetShader(NULL, NULL, 0);

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
	// Initialise the static model storage
	VolLineShader::BaseModel = NULL;
	VolLineShader::LineModels.clear();

	// Create a new static base model for non-textured lines
	VolLineShader::BaseModel = VolLineShader::CreateLineModel(NULL);

	// Load the linear depth texture as a static resource for each rendering cycle
	VolLineShader::LinearDepthTextureObject = new Texture();
	Result result = VolLineShader::LinearDepthTextureObject->Initialise(concat(D::IMAGE_DATA)("\\Rendering\\linear_depth.dds").str().c_str());
	if (result != ErrorCodes::NoError) return ErrorCodes::CouldNotInitialiseStaticVolumetricLineData;
	VolLineShader::LinearDepthTexture = VolLineShader::LinearDepthTextureObject->GetTexture();
	
	// We will maintain a two-resource array as input parameter for textured line rendering.  The second parameter
	// will be filled at runtime by the shader texture required
	PSShaderResources = new ID3D11ShaderResourceView*[2];
	PSShaderResources[0] = VolLineShader::LinearDepthTexture;
	PSShaderResources[1] = NULL;

	// Return success
	return ErrorCodes::NoError;
}

// Returns a model appropriate for rendering volumetric lines with the specified texture, or for pure
// non-textured volumetric lines if render_texture == NULL
ModelBuffer * VolLineShader::LineModel(Texture *render_texture)
{
	// If we want pure volumetric line rendering then simply return the base model
	if (!render_texture) return VolLineShader::BaseModel;

	// Otherwise check whether we already have a model for this render texture
	if (VolLineShader::LineModels.count(render_texture) == 0)
	{
		// We do not have a model for this texture, so create one now before returning it
		ModelBuffer *buffer = VolLineShader::CreateLineModel(render_texture);
		LineModels[render_texture] = buffer;
		return buffer;
	}
	else
	{
		// We already have a model for this texture, so simply return it here
		return LineModels[render_texture];
	}
}

// Creates a new line model appropriate for rendering volumetric lines with the specified texture, or for pure
// non-textured volumetric lines if render_texture == NULL
ModelBuffer * VolLineShader::CreateLineModel(Texture *render_texture)
{
	ModelBuffer *buffer = new ModelBuffer();
	if (!buffer) return NULL;

	// Create the base vertex and index data
	XMFLOAT3 *v = new XMFLOAT3[1] { XMFLOAT3(0.0f, 0.0f, 0.0f) }; 
	UINT16 *i = new UINT16[1] { 0U }; 

	// Initialise the base model using this data
	Result result = buffer->Initialise(Game::Engine->GetDevice(), (const void**)&v, sizeof(XMFLOAT3), 1U, (const void**)&i, sizeof(UINT16), 1U);
	if (result != ErrorCodes::NoError) { delete(v); delete(i); delete(buffer); return NULL; }

	// Assign the base model texture
	result = buffer->SetTexture(render_texture);
	if (result != ErrorCodes::NoError) { delete(v); delete(i); delete(buffer); return NULL; }

	// Deallocate the model vertex and index data
	SafeDelete(v);
	SafeDelete(i);

	// Return the new line model
	return buffer;
}








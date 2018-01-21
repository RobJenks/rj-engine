#include <unordered_map>
#include "DX11_Core.h"
#include "Utility.h"

#include "CoreEngine.h"
#include "RenderAssetsDX11.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "ModelBuffer.h"
#include "CoreEngine.h"
#include "TextureDX11.h"
#include "VolumetricLine.h"

#include "VolLineShader.h"

// Static resources used during line rendering
ModelBuffer *										VolLineShader::BaseModel = NULL;
std::unordered_map<std::string, ModelBuffer*>		VolLineShader::LineModels;
TextureDX11 *										VolLineShader::LinearDepthTextureObject = NULL;
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


Result VolLineShader::Initialise(Rendering::RenderDeviceType  *device, XMFLOAT2 viewport_size, float clip_near, float clip_far)
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
Result VolLineShader::InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename)
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
Result VolLineShader::InitialiseGeometryShader(Rendering::RenderDeviceType  *device, std::string filename)
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
Result VolLineShader::InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename)
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
Result VolLineShader::InitialisePixelShaderTextured(Rendering::RenderDeviceType  *device, std::string filename)
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
Result XM_CALLCONV VolLineShader::Render(	Rendering::RenderDeviceContextType  *deviceContext, unsigned int vertexCount, unsigned int indexCount, unsigned int instanceCount,
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
Result VolLineShader::InitialiseStaticData(Rendering::RenderDeviceType  *device)
{
	// Initialise the static model storage
	VolLineShader::BaseModel = NULL;
	VolLineShader::LineModels.clear();

	// Create a new static base model for non-textured lines
	VolLineShader::BaseModel = VolLineShader::CreateLineModel(NULL);

	// Load the linear depth texture as a static resource for each rendering cycle
	VolLineShader::LinearDepthTextureObject = Game::Engine->GetAssets().GetTexture("linear_depth");
	VolLineShader::LinearDepthTexture = VolLineShader::LinearDepthTextureObject->GetShaderResourceView();
	
	// We will maintain a two-resource array as input parameter for textured line rendering.  The second parameter
	// will be filled at runtime by the shader texture required
	PSShaderResources = new ID3D11ShaderResourceView*[2];
	PSShaderResources[0] = VolLineShader::LinearDepthTexture;
	PSShaderResources[1] = NULL;

	// Return success
	return ErrorCodes::NoError;
}

// Returns a model appropriate for rendering volumetric lines with the specified material, or for pure
// non-textured volumetric lines if render_material == NULL
ModelBuffer * VolLineShader::LineModel(MaterialDX11 *render_material)
{
	// If we want pure volumetric line rendering then simply return the base model
	if (!render_material) return VolLineShader::BaseModel;

	// Otherwise check whether we already have a model for this render texture
	const std::string & name = render_material->GetCode();
	if (VolLineShader::LineModels.count(name) == 0)
	{
		// We do not have a model for this texture, so create one now before returning it
		ModelBuffer *buffer = VolLineShader::CreateLineModel(render_material);
		LineModels[name] = buffer;
		return buffer;
	}
	else
	{
		// We already have a model for this texture, so simply return it here
		return LineModels[name];
	}
}

// Creates a new line model appropriate for rendering volumetric lines with the specified material, or for pure
// non-textured volumetric lines if render_material == NULL
ModelBuffer * VolLineShader::CreateLineModel(MaterialDX11 *render_material)
{
	// Create the base vertex and index data
	XMFLOAT3 *v = new XMFLOAT3[1] { XMFLOAT3(0.0f, 0.0f, 0.0f) }; 
	UINT32 *i = new UINT32[1] { 0U }; 

	// Initialise the base model using this data
	ModelBuffer *buffer = new ModelBuffer((const void**)&v, sizeof(XMFLOAT3), 1U, (const void**)&i, sizeof(UINT32), 1U, render_material);
	if (!buffer) { delete(v); delete(i); delete(buffer); return NULL; }

	// Deallocate the model vertex and index data
	SafeDeleteArray(v);
	SafeDeleteArray(i);

	// Return the new line model
	return buffer;
}

// Deallocates all static data used in volumetric line rendering
void VolLineShader::ShutdownStaticData(void)
{
	// Deallocate each standard line model in turn
	std::unordered_map<std::string, ModelBuffer*>::iterator it_end = VolLineShader::LineModels.end();
	for (std::unordered_map<std::string, ModelBuffer*>::iterator it = VolLineShader::LineModels.begin(); it != it_end; ++it)
	{
		if (it->second) delete(it->second);
	}
	VolLineShader::LineModels.clear();

	// Deallocate the static base model for pure volumetric rendering 
	SafeDelete(VolLineShader::BaseModel);

	// Deallocate the cached rendering resource array
	SafeDeleteArray(VolLineShader::PSShaderResources);
}






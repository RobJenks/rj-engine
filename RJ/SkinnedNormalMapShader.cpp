////////////////////////////////////////////////////////////////////////////////
// Filename: SkinnedNormalMapShader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "FastMath.h"
#include "iShader.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "ShaderManager.h"
#include "InputLayoutDesc.h"
#include "LightData.hlsl.h"
#include "Data\\Shaders\\standard_ps_const_buffer.h"

#include "SkinnedNormalMapShader.h"


SkinnedNormalMapShader::SkinnedNormalMapShader(void)
{
	// Set pointers to NULL
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_inputlayout = 0;
	m_perFrameBuffer = 0;
	m_perObjectBuffer = 0;
	m_perSubsetBuffer = 0;
}


Result SkinnedNormalMapShader::Initialise(Rendering::RenderDeviceType * device, HWND hwnd)
{
	Result result;

	// Initialise each shader in turn
	result = InitialiseVertexShader(device, iShader::ShaderFilename("skinned_nmap.vs.cso"));
	if (result != ErrorCodes::NoError) return result;

	result = InitialisePixelShader(device, iShader::ShaderFilename("skinned_nmap.ps.cso"));
	if (result != ErrorCodes::NoError) return result;

	// Initialise all constant buffers required for rendering
	result = InitialiseCommonConstantBuffers(device);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result SkinnedNormalMapShader::InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateSkinnedNMShaderVS;

	// Define the input layout for this vertex shader
	InputLayoutDesc layout_desc = InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0);

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreateVertexShader(device, filename, &layout_desc, &m_vertexShader, &m_inputlayout);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingSkinnedNMShaderVS;

	// Return success
	return ErrorCodes::NoError;
}


// Initialise shader
Result SkinnedNormalMapShader::InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename)
{
	Result result;

	// Parameter check
	if (!device || filename == NullString) return ErrorCodes::NullInputToCreateSkinnedNMShaderPS;

	// Attempt to load and create the compiled shader 
	result = ShaderManager::CreatePixelShader(device, filename, &m_pixelShader);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingSkinnedNMShaderPS;

	// Return success
	return ErrorCodes::NoError;
}

// Initialise constant buffers used across all pipeline shaders
Result SkinnedNormalMapShader::InitialiseCommonConstantBuffers(Rendering::RenderDeviceType  *device)
{
	Result result;

	// Parameter check
	if (!device) return ErrorCodes::NullInputToCreateSkinnedNMConstBuffers;

	// Create the per-frame (PS) shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(StandardPSConstBuffer), device, &m_perFrameBuffer);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingSkinnedNMShaderConstBuffers;

	// Create the per-object (VS) shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(PerObjectBuffer), device, &m_perObjectBuffer);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingSkinnedNMShaderConstBuffers;

	// Create the per-subset shader constant buffer
	result = ShaderManager::CreateStandardDynamicConstantBuffer(sizeof(PerSubsetBuffer), device, &m_perSubsetBuffer);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorCreatingSkinnedNMShaderConstBuffers;

	// Return success
	return ErrorCodes::NoError;
}


void SkinnedNormalMapShader::Shutdown()
{
	// Release all resources
	ReleaseIfExists(m_perFrameBuffer);
	ReleaseIfExists(m_perObjectBuffer);
	ReleaseIfExists(m_perSubsetBuffer);
	ReleaseIfExists(m_inputlayout);
	ReleaseIfExists(m_vertexShader);
	ReleaseIfExists(m_pixelShader);
}

Result RJ_XM_CALLCONV SkinnedNormalMapShader::Render(Rendering::RenderDeviceContextType  *deviceContext, SkinnedModelInstance &model,
									  XMFLOAT3 eyepos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	Result result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerSubsetBuffer *cbSubset;
	ID3D11ShaderResourceView *SRVs[2];

	// TODO: TEMP: Set the per-frame shader parameters here for now, for simplicity
	result = SetPerFrameShaderParameters(deviceContext);
	if (result != ErrorCodes::NoError) return result;

	// Set the per-object shader parameters that we will use for rendering
	result = SetPerObjectShaderParameters(deviceContext, model, viewMatrix, projectionMatrix);
	if(result != ErrorCodes::NoError) return result;


	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_inputlayout);

	// Set the vertex and pixel shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Render each subset of the model in turn
	for (UINT subset = 0; subset < model.Model->SubsetCount; ++subset)
	{
		// We have to update the material buffer with the material for this subset
		result = deviceContext->Map(m_perSubsetBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result)) return ErrorCodes::SkinnedNormalMapSubsetRenderingFailure;

		// Update the material
		cbSubset = (PerSubsetBuffer*)mappedResource.pData;
		cbSubset->gMaterial = model.Model->Mat[subset];

		// Unlock and set the constant buffer
		deviceContext->Unmap(m_perSubsetBuffer, 0);
		deviceContext->PSSetConstantBuffers((unsigned int)1, 1, &m_perSubsetBuffer);

		// Send the texture resources corresponding to this subset of the mesh
		SRVs[0] = model.Model->DiffuseMapSRV[subset];
		SRVs[1] = model.Model->NormalMapSRV[subset];
		deviceContext->PSSetShaderResources(0, 2, SRVs);

		// Finally, send the subset vertices through the shader pipeline for rendering
		model.Model->ModelMesh.Draw(deviceContext, subset);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Set the per-frame shader parameters in this method.  This includes only PS parameters
Result SkinnedNormalMapShader::SetPerFrameShaderParameters(Rendering::RenderDeviceContextType  *deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	StandardPSConstBuffer *cbFrameBuffer;

	/* *** Per-frame PS constant buffer *** */
	result = deviceContext->Map(m_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) return ErrorCodes::CouldNotObtainShaderBufferLock;

	// Get a pointer to the data in the constant buffer and copy data into it
	cbFrameBuffer = (StandardPSConstBuffer*)mappedResource.pData;
	{
		Game::Engine->ShaderManager.PopulateConstantBuffer(cbFrameBuffer);
	}
	
	// Unlock the constant buffer.
	deviceContext->Unmap(m_perFrameBuffer, 0);

	/* Set these buffers and resources for consumption by each shader */

	// Set pixel shader constant buffers
	deviceContext->PSSetConstantBuffers((unsigned int)0, 1, &m_perFrameBuffer);

	// Return success
	return ErrorCodes::NoError;
}

// Set the per-object parameters in this method.  This includes the per-object CB as well as the bone transform CB
Result SkinnedNormalMapShader::SetPerObjectShaderParameters(Rendering::RenderDeviceContextType  *deviceContext, SkinnedModelInstance &model, 
															XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerObjectBuffer *cbObjPtr;

	// Static matrix that transforms NDC space [-1,+1]^2 to texture space [0,1]^2
	static XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// Transpose the matrices to prepare them for the shader
	/*D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);*/

	// Load the basic matrices into SIMD-aligned structures for input to the shader
	XMMATRIX world			= XMLoadFloat4x4(&model.World);
	XMMATRIX view			= XMLoadFloat4x4(&viewMatrix);
	XMMATRIX proj			= XMLoadFloat4x4(&projectionMatrix);
	XMMATRIX viewproj		= XMMatrixMultiply(view, proj);
	XMMATRIX worldviewproj	= world * view * proj;

	/* *** Per-object VS constant buffer *** */
	result = deviceContext->Map(m_perObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) return ErrorCodes::CouldNotObtainShaderBufferLock;

	// Get a pointer to the data in the constant buffer and copy data into it
	cbObjPtr = (PerObjectBuffer*)mappedResource.pData;
	cbObjPtr->gWorld = XMMatrixTranspose( world );
	cbObjPtr->gWorldInvTranspose = XMMatrixTranspose( MatrixInverseTranspose(world) );
	cbObjPtr->gWorldViewProj = XMMatrixTranspose( worldviewproj );
	cbObjPtr->gWorldViewProjTex =XMMatrixTranspose( worldviewproj * toTexSpace );
	cbObjPtr->gShadowTransform =XMMatrixTranspose( (world /** shadowTransform*/) );
	cbObjPtr->gTexTransform = XMMatrixTranspose( XMMatrixScaling(1.0f, 1.0f, 1.0f) );

	// Also set the bone transform data for this instance of the model
	memcpy(cbObjPtr->gBoneTransforms, &(model.FinalTransforms[0]), sizeof(XMFLOAT4X4) * model.FinalTransforms.size());

	// Unlock the constant buffer.
	deviceContext->Unmap(m_perObjectBuffer, 0);

	/* Set the buffer for consumption by the vertex shader */

	// Set vertex shader constant buffers
	deviceContext->VSSetConstantBuffers((unsigned int)0, 1, &m_perObjectBuffer);

	/* No pixel shader buffers or resources to be set on a per-object basis */

	// Return success
	return ErrorCodes::NoError;
}


SkinnedNormalMapShader::~SkinnedNormalMapShader()
{
}

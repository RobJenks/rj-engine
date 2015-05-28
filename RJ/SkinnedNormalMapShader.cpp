////////////////////////////////////////////////////////////////////////////////
// Filename: SkinnedNormalMapShader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "FastMath.h"
#include "DXLocaliser.h"

#include "SkinnedNormalMapShader.h"


SkinnedNormalMapShader::SkinnedNormalMapShader(const DXLocaliser *locale)
{
	// Store a reference to the current locale
	m_locale = locale;

	// Set pointers to NULL
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_perFrameBuffer = 0;
	m_perObjectBuffer = 0;
	m_perSubsetBuffer = 0;

	// TODO: TEMP: Create temporary directional lights for now.  Set by light update method in future
	m_lights = new DirectionalLight[3];
	m_lights[0] = DirectionalLight(XMFLOAT3(-0.57735f, -0.57735f, 0.57735f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f), XMFLOAT4

(0.8f, 0.8f, 0.7f, 1.0f));
	m_lights[1] = DirectionalLight(XMFLOAT3(0.707f, -0.707f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f), XMFLOAT4(0.2f, 

0.2f, 0.2f, 1.0f));
	m_lights[2] = DirectionalLight(XMFLOAT3(0.0f, 0.0, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.2f, 0.2f, 0.2f, 

1.0f));
}


SkinnedNormalMapShader::SkinnedNormalMapShader(const SkinnedNormalMapShader& other)
{
}


SkinnedNormalMapShader::~SkinnedNormalMapShader()
{
}


Result SkinnedNormalMapShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	Result result;

	// Initialise the vertex and pixel shaders, depending on the current DX locale
	if (m_locale->Locale.ShaderModelLevel == DXLocaliser::SMLevel::SM_5_0)		
	{
		// Initialise a Shader Model 5 shader
		result = InitialiseShader_SM5(device, hwnd, iShader::ShaderFilename("skinned_nmap_sm_5_0.hlsl").c_str(), 
													iShader::ShaderFilename("skinned_nmap_sm_5_0.hlsl").c_str());
	}
	else 
	{
		// TODO: *** SKINNED MODEL RENDERING MAY CURRENTLY REQUIRE SM5.0.  NEED TO DETERMINE IF TRUE, AND/OR
		// HOW IT CAN BE IMPLEMENTED TO SUPPORT LOWER SHADER MODEL LEVELS ***
		return ErrorCodes::CouldNotInitialiseShaderToUnsupportedModel;

		// Initialise a Shader Model 2 shader
		result = InitialiseShader_SM2(device, hwnd, iShader::ShaderFilename("light_sm_2_0.vs").c_str(), 
													iShader::ShaderFilename("light_sm_2_0.ps").c_str());
	}
	
	// If shader initialisation failed then return the error code here
	if (result != ErrorCodes::NoError) return result;

	// Otherwise return success
	return ErrorCodes::NoError;
}


void SkinnedNormalMapShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

Result SkinnedNormalMapShader::Render(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model,
									  XMFLOAT3 eyepos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	Result result;

	// TODO: TEMP: Set the per-frame shader parameters here for now, for simplicity
	result = SetPerFrameShaderParameters(deviceContext, m_lights, eyepos);
	if (result != ErrorCodes::NoError) return result;

	// Set the per-object shader parameters that we will use for rendering
	result = SetPerObjectShaderParameters(deviceContext, model, viewMatrix, projectionMatrix);
	if(result != ErrorCodes::NoError) return result;

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, model);

	return ErrorCodes::NoError;
}

Result SkinnedNormalMapShader::InitialiseShader_SM5(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	unsigned int numElements;
	D3D11_BUFFER_DESC perFrameBufferDesc;
	D3D11_BUFFER_DESC perObjectBufferDesc;
	D3D11_BUFFER_DESC perSubsetBufferDesc;

	// Initialise the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile((LPCSTR)vsFilename, NULL, NULL, "SkinnedVertexShader", 
									m_locale->Locale.VertexShaderLevelDesc, 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								    &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);	
			return ErrorCodes::SkinnedVertexShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			return ErrorCodes::SkinnedVertexShaderFileMissing;
		}
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile((LPCSTR)psFilename, NULL, NULL, "SkinnedPixelShader", 
									m_locale->Locale.PixelShaderLevelDesc, 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								    &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			return ErrorCodes::SkinnedPixelShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			return ErrorCodes::SkinnedPixelShaderFileMissing;
		}
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateSkinnedVertexShader;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateSkinnedPixelShader;
	}

	// Create the vertex input layout description
    const D3D11_INPUT_ELEMENT_DESC polygonLayout[] =
	{
		{"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES",  0, DXGI_FORMAT_R8G8B8A8_UINT,   0, 60,		D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderInputLayout;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	//// Create a texture sampler state description.
 //   samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
 //   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
 //   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
 //   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
 //   samplerDesc.MipLODBias = 0.0f;
 //   samplerDesc.MaxAnisotropy = 1;
 //   samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
 //   samplerDesc.BorderColor[0] = 0;
	//samplerDesc.BorderColor[1] = 0;
	//samplerDesc.BorderColor[2] = 0;
	//samplerDesc.BorderColor[3] = 0;
 //   samplerDesc.MinLOD = 0;
 //   samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//// Create the texture sampler state.
 //   result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	//if(FAILED(result))
	//{
	//	return ErrorCodes::CouldNotCreateVertexShaderSamplerState;
	//}

	// Setup the description of the dynamic per-frame constant buffer used in the shaders
	perFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameBufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    perFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    perFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    perFrameBufferDesc.MiscFlags = 0;
	perFrameBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	size_t sz = sizeof(PerFrameBuffer);
	result = device->CreateBuffer(&perFrameBufferDesc, NULL, &m_perFrameBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderPerFrameConstBuffer;
	}

	// Setup the description of the per-object constant buffer used in the shaders
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	perObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectBufferDesc.ByteWidth = sizeof(PerObjectBuffer);
	perObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectBufferDesc.MiscFlags = 0;
	perObjectBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&perObjectBufferDesc, NULL, &m_perObjectBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderPerObjectConstBuffer;
	}

	// Setup the description of the per-subset constant buffer used in the shaders
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	perSubsetBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perSubsetBufferDesc.ByteWidth = sizeof(PerSubsetBuffer);
	perSubsetBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perSubsetBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perSubsetBufferDesc.MiscFlags = 0;
	perSubsetBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&perSubsetBufferDesc, NULL, &m_perSubsetBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderPerSubsetConstBuffer;
	}

	// Return success if we have created all the components above
	return ErrorCodes::NoError;
}

Result SkinnedNormalMapShader::InitialiseShader_SM2(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	/*HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;


	// Initialise the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile((LPCSTR)vsFilename, NULL, NULL, "LightVertexShader", 
									m_locale->Locale.VertexShaderLevelDesc,
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								    &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			return ErrorCodes::LightVertexShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			return ErrorCodes::LightVertexShaderFileMissing;
		}
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile((LPCSTR)psFilename, NULL, NULL, "LightPixelShader", 
									m_locale->Locale.PixelShaderLevelDesc, 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								    &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			return ErrorCodes::LightPixelShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			return ErrorCodes::LightPixelShaderFileMissing;
		}
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateLightVertexShader;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateLightPixelShader;
	}

	// Create the vertex input layout description.  Definition incorporates instancing as per the iShader interface spec
    const D3D11_INPUT_ELEMENT_DESC polygonLayout[] =
    {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,								

D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "mTransform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,								

D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "mTransform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "mTransform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                               &m_layout);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderInputLayout;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderSamplerState;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderMatConstBuffer;
	}

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderLightConstBuffer;
	}
*/
	// Return success if we have created all the components above
	return ErrorCodes::NoError;
}


void SkinnedNormalMapShader::ShutdownShader()
{
	// Release the per-subset constant buffer.
	if(m_perSubsetBuffer)
	{
		m_perSubsetBuffer->Release();
		m_perSubsetBuffer = 0;
	}

	// Release the per-object constant buffer.
	if(m_perObjectBuffer)
	{
		m_perObjectBuffer->Release();
		m_perObjectBuffer = 0;
	}

	// Release the per-frame constant buffer.
	if(m_perFrameBuffer)
	{
		m_perFrameBuffer->Release();
		m_perFrameBuffer = 0;
	}

	// Release the sampler state.
	/*if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}*/

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}


void SkinnedNormalMapShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	return;
}

// Set the per-frame shader parameters in this method.  This includes only PS parameters
Result SkinnedNormalMapShader::SetPerFrameShaderParameters(ID3D11DeviceContext *deviceContext, DirectionalLight *lights3, XMFLOAT3 eyepos)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerFrameBuffer *cbFrameBuffer;

	/* *** Per-frame PS constant buffer *** */
	result = deviceContext->Map(m_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) return ErrorCodes::CouldNotObtainShaderBufferLock;

	// Get a pointer to the data in the constant buffer and copy data into it
	cbFrameBuffer = (PerFrameBuffer*)mappedResource.pData;
	memcpy(cbFrameBuffer->gDirLights, lights3, sizeof(DirectionalLight) * 3);	// Hardcoded to always support three lights (even if set to zero)
	cbFrameBuffer->gEyePosW = eyepos;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_perFrameBuffer, 0);

	/* Set these buffers and resources for consumption by each shader */

	// Set pixel shader constant buffers
	deviceContext->PSSetConstantBuffers((unsigned int)0, 1, &m_perFrameBuffer);

	// Return success
	return ErrorCodes::NoError;
}

// Set the per-object parameters in this method.  This includes the per-object CB as well as the bone transform CB
Result SkinnedNormalMapShader::SetPerObjectShaderParameters(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model, 
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


void SkinnedNormalMapShader::RenderShader(ID3D11DeviceContext *deviceContext, SkinnedModelInstance &model)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerSubsetBuffer *cbSubset;
	ID3D11ShaderResourceView *SRVs[2];

	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this model
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	//deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render each subset of the model in turn
	for(UINT subset = 0; subset < model.Model->SubsetCount; ++subset)
	{
		// We have to update the material buffer with the material for this subset
		result = deviceContext->Map(m_perSubsetBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if(FAILED(result)) return;		// TODO: Error handling

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
}	

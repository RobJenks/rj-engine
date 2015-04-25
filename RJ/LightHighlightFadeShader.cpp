////////////////////////////////////////////////////////////////////////////////
// Filename: LightHighlightFadeShader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "DXLocaliser.h"

#include "LightHighlightFadeShader.h"


LightHighlightFadeShader::LightHighlightFadeShader(const DXLocaliser *locale)
{
	// Store a reference to the current locale
	m_locale = locale;

	// Set pointers to NULL
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	m_lightBuffer = 0;
}


LightHighlightFadeShader::LightHighlightFadeShader(const LightHighlightFadeShader& other)
{
}


LightHighlightFadeShader::~LightHighlightFadeShader()
{
}


Result LightHighlightFadeShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	Result result;

	// Initialise the vertex and pixel shaders.  This shader is common to all SM levels so no different logic is required.
	if (m_locale->DXL_SM_LEVEL == DXLocaliser::SMLevel::SM_5_0)
	{
		// Initialise a Shader Model 5 shader
		result = InitialiseShader_SM_All(device, hwnd, "../RJ/Data/Shaders/light_highlight_fade_sm_all.vs",
			"../RJ/Data/Shaders/light_highlight_fade_sm_all.ps");
	}
	else if (m_locale->DXL_SM_LEVEL == DXLocaliser::SMLevel::SM_2_0)
	{
		// Initialise a Shader Model 2 shader
		result = InitialiseShader_SM_All(device, hwnd, "../RJ/Data/Shaders/light_highlight_fade_sm_all.vs",
			"../RJ/Data/Shaders/light_highlight_fade_sm_all.ps");
	}
	else { return ErrorCodes::CouldNotInitialiseShaderToUnsupportedModel; }

	// If shader initialisation failed then return the error code here
	if (result != ErrorCodes::NoError) return result;

	// Otherwise return success
	return ErrorCodes::NoError;
}


void LightHighlightFadeShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

Result LightHighlightFadeShader::Render(ID3D11DeviceContext *deviceContext, int vertexCount, int indexCount, int instanceCount,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	Result result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, viewMatrix, projectionMatrix, texture);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, vertexCount, indexCount, instanceCount);

	return ErrorCodes::NoError;
}

// Sets the parameters specific to the light shader, i.e. light type / direction / colour
Result LightHighlightFadeShader::SetLightParameters(D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor)
{
	// Store the new light/alpha parameters; these will take effect in the next call to SetShaderParameters (each frame)
	m_lightdirection = lightDirection;
	m_ambientcolour = ambientColor;
	m_diffusecolour = diffuseColor;

	// Return success
	return ErrorCodes::NoError;
}

Result LightHighlightFadeShader::InitialiseShader_SM_All(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
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
	result = D3DX11CompileFromFile((LPCSTR)vsFilename, NULL, NULL, "LightHighlightFadeVertexShader",
		m_locale->DXL_VERTEX_SHADER_LEVEL_S,
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			return ErrorCodes::LightHighlightFadeVertexShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			return ErrorCodes::LightHighlightFadeVertexShaderFileMissing;
		}
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile((LPCSTR)psFilename, NULL, NULL, "LightHighlightFadePixelShader",
		m_locale->DXL_PIXEL_SHADER_LEVEL_S,
		D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			return ErrorCodes::LightHighlightFadePixelShaderCompilationFailed;
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			return ErrorCodes::LightHighlightFadePixelShaderFileMissing;
		}
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return ErrorCodes::CannotCreateLightHighlightFadeVertexShader;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return ErrorCodes::CannotCreateLightHighlightFadePixelShader;
	}

	// Create the vertex input layout description.  Definition incorporates instancing as per the iShader interface spec
	const D3D11_INPUT_ELEMENT_DESC polygonLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "mTransform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "mTransform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "mTransform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "mTransform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "iParams", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&m_layout);
	if (FAILED(result))
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
	if (FAILED(result))
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
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderMatConstBuffer;
	}

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightHighlightFadeBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotCreatePixelShaderLightConstBuffer;
	}

	// Return success if we have created all the components above
	return ErrorCodes::NoError;
}

void LightHighlightFadeShader::ShutdownShader()
{
	// Release the light constant buffer.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}


void LightHighlightFadeShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
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
	for (i = 0; i<bufferSize; i++)
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


Result LightHighlightFadeShader::SetShaderParameters(ID3D11DeviceContext *deviceContext, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
	ID3D11ShaderResourceView* texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightHighlightFadeBufferType* dataPtr2;

	// Transpose the matrices to prepare them for the shader
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotObtainShaderBufferLock;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return ErrorCodes::CouldNotObtainLightHighlightFadeShaderBufferLock;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightHighlightFadeBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->ambientColor = m_ambientcolour;
	dataPtr2->diffuseColor = m_diffusecolour;
	dataPtr2->lightDirection = m_lightdirection;
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return ErrorCodes::NoError;
}


void LightHighlightFadeShader::RenderShader(ID3D11DeviceContext *deviceContext, int vertexCount, int indexCount, int instanceCount)
{
	// Set the vertex input layout
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this model
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the model
	deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}

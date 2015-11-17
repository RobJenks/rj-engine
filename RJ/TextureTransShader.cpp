#include "ErrorCodes.h"
#include "DXLocaliser.h"

#include "TextureTransShader.h"


Result TextureTransShader::Initialise(ID3D11Device* device, HWND hwnd)
{
	// Attempt to initialise the shader from vs/ps files.  Note that there are no methods tailored per shader
	// model level, as in many other shaders, since all these functions are supported in SM2 and SM5
	Result result = InitialiseShader(device, hwnd,	iShader::ShaderFilename("texturetrans_sm_all.vs").c_str(),
													iShader::ShaderFilename("texturetrans_sm_all.ps").c_str());

	// Return the result of the initialisation
	return result;
}

void TextureTransShader::Shutdown(void)
{
	// Shut down the shader components
	ShutdownShader();
}


Result TextureTransShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, const FXMMATRIX worldMatrix, 
								const CXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, 
								float transX, float transY)
{
	// Set the shader parameters that the shader will use for rendering.
	Result result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, transX, transY);
	if(result != ErrorCodes::NoError) return result;
	
	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	// Signal success
	return ErrorCodes::NoError;
}

Result TextureTransShader::InitialiseShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC translateBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TextureTransVertexShader", m_locale->Locale.VertexShaderLevelDesc, 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
			return ErrorCodes::TextureVertexShaderCompilationFailed;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			return ErrorCodes::TextureVertexShaderFileMissing;
		}
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TextureTransPixelShader", m_locale->Locale.PixelShaderLevelDesc, 
									D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
			return ErrorCodes::TexturePixelShaderCompilationFailed;
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			return ErrorCodes::TexturePixelShaderFileMissing;
		}
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, 
										 &m_vertexShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateTextureVertexShader;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, 
									   &m_pixelShader);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateTexturePixelShader;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
									   vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateVertexShaderInputLayout;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

    // Setup the description of the dynamic constant buffer that is in the vertex shader.
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
		return ErrorCodes::CouldNotCreateVertexShaderConstantBuffer;
	}

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

    // Setup the description of the texture translation dynamic constant buffer that is in the pixel shader.
    translateBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	translateBufferDesc.ByteWidth = sizeof(TranslateBufferType);
    translateBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    translateBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    translateBufferDesc.MiscFlags = 0;
	translateBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&translateBufferDesc, NULL, &m_translateBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreatePixelShaderConstantBuffer;
	}

	// Return success
	return ErrorCodes::NoError;
}

void TextureTransShader::ShutdownShader()
{
	// Release the texture translation constant buffer.
	if(m_translateBuffer)
	{
		m_translateBuffer->Release();
		m_translateBuffer = 0;
	}

	// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

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
}

void TextureTransShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
{
	char* compileErrors;
	SIZE_T bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i = 0; i < bufferSize; ++i)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;
}

Result TextureTransShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, const FXMMATRIX worldMatrix, const CXMMATRIX viewMatrix, 
											   const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, 
											   float transX, float transY)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;
	TranslateBufferType* dataPtr2;

	// Lock the matrix constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotObtainShaderBufferLock;
	}

	// Get a pointer to the data in the matrix constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Transpose and copy the matrices into the matrix constant buffer.
	XMStoreFloat4x4(&dataPtr->world, XMMatrixTranspose(worldMatrix));
	XMStoreFloat4x4(&dataPtr->view, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&dataPtr->projection, XMMatrixTranspose(projectionMatrix));

	// Unlock the buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the matrix constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Lock the texture translation constant buffer so it can be written to.
	result = deviceContext->Map(m_translateBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotObtainShaderCustomBufferLock;
	}

	// Get a pointer to the data in the texture translation constant buffer.
	dataPtr2 = (TranslateBufferType*)mappedResource.pData;

	// Copy the translation value into the texture translation constant buffer.
	dataPtr2->TranslateX = transX;
	dataPtr2->TranslateY = transY;

	// Unlock the buffer.
    deviceContext->Unmap(m_translateBuffer, 0);

	// Set the position of the texture translation constant buffer in the pixel shader.
	bufferNumber = 0;

	// Now set the texture translation constant buffer in the pixel shader with the updated values.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_translateBuffer);

	// Return success
	return ErrorCodes::NoError;
}

void TextureTransShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}



TextureTransShader::TextureTransShader(const DXLocaliser *locale) 
{
	// Store a reference to the current locale
	m_locale = locale;

	// Set pointers to null before we attempt initialisation
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_translateBuffer = 0;
}
TextureTransShader::TextureTransShader(const TextureTransShader &other) { }
TextureTransShader::~TextureTransShader(void) { }

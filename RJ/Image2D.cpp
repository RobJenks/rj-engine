#include "ErrorCodes.h"
#include "CoreEngine.h"
#include "GameVarsExtern.h"
#include "Texture.h"

#include "Image2D.h"

Image2D::Image2D()
{
	m_render = true;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
	m_vertices = 0;
	m_x = m_y = 0;
	m_z = 0.0f;
}


Image2D::Image2D(const Image2D& other)
{
}


Image2D::~Image2D()
{
}


Result Image2D::Initialize(ID3D11Device* device, int screenWidth, int screenHeight, const char *textureFilename, int bitmapWidth, int bitmapHeight)
{ 
	Result result;

	// Store the screen size.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_screenHalfWidth = (float)m_screenWidth / 2.0f;
	m_screenHalfHeight = (float)m_screenHeight / 2.0f;
	m_screenLeft = (m_screenHalfWidth * -1.0f);

	// Store the size in pixels that this bitmap should be rendered at.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	// Initialize the previous rendering position to negative one.
	m_previousPosX = -1;
	m_previousPosY = -1;
	m_previousPosZ = -1.0f;

	// Store a reference to the device and device context
	m_device = Game::Engine->GetDevice();
	m_devicecontext = Game::Engine->GetDeviceContext();

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Load the texture for this bitmap.
	result = LoadTexture(device, textureFilename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	return ErrorCodes::NoError;
}


void Image2D::Shutdown()
{
	// Release the bitmap texture.
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();
}


void Image2D::Render(int positionX, int positionY, float zOrder)
{
	// Set the internal position before rendering
	m_x = positionX;
	m_y = positionY;
	m_z = zOrder;

	// Now call the main rendering function
	Render();
}

void Image2D::Render(void)
{
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	Result result = UpdateBuffers();
	if(result != ErrorCodes::NoError)
	{
		return;
	}

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();
}

Result Image2D::InitializeBuffers(ID3D11Device* device)
{
	INDEXFORMAT *indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Set the number of vertices in the vertex array.
	m_vertexCount = 6;

	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	m_vertices = new VertexType[m_vertexCount];
	if(!m_vertices)
	{
		return ErrorCodes::CouldNotAllocateImage2DVertexArray;
	}

	// Create the index array.
	indices = new INDEXFORMAT[m_indexCount];
	if(!indices)
	{
		return ErrorCodes::CouldNotAllocateImage2DIndexArray;
	}

	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// Load the vertex array with constant data, e.g. texture coords, that we do not need to set each frame
	// First triangle.
	m_vertices[0].texture = XMFLOAT2(0.0f, 0.0f);			// Top left
	m_vertices[1].texture = XMFLOAT2(1.0f, 1.0f);			// Bottom right
	m_vertices[2].texture = XMFLOAT2(0.0f, 1.0f);			// Bottom left

	// Second triangle.
	m_vertices[3].texture = XMFLOAT2(0.0f, 0.0f);			// Top left
	m_vertices[4].texture = XMFLOAT2(1.0f, 0.0f);			// Top right
	m_vertices[5].texture = XMFLOAT2(1.0f, 1.0f);			// Bottom right

	// Load the index array with data.
	for(i=0; i<m_indexCount; i++) indices[i] = i;

	// Set up the description of the DYNAMIC vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateImage2DVertexBuffer;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(INDEXFORMAT) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateImage2DIndexBuffer;
	}

	// Release the index array now that the buffer has been created and loaded.
	delete [] indices;
	indices = 0;

	return ErrorCodes::NoError;
}


void Image2D::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	// Release the vertex array
	if (m_vertices)
	{
		delete m_vertices;
		m_vertices = 0;
	}
}


Result Image2D::UpdateBuffers(void)
{
	float left, right, top, bottom;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;


	// If the position we are rendering this bitmap to has not changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	if((m_x == m_previousPosX) && (m_y == m_previousPosY) && (m_z == m_previousPosZ))
	{
		return ErrorCodes::NoError;
	}
	
	// If it has changed then update the position it is being rendered to.
	m_previousPosX = m_x;
	m_previousPosY = m_y;
	m_previousPosZ = m_z;

	// Calculate the screen coordinates of the left side of the bitmap.
	left = m_screenLeft + (float)m_x;

	// Calculate the screen coordinates of the right side of the bitmap.
	right = left + (float)m_bitmapWidth;

	// Calculate the screen coordinates of the top of the bitmap.
	top = m_screenHalfHeight - (float)m_y;

	// Calculate the screen coordinates of the bottom of the bitmap.
	bottom = top - (float)m_bitmapHeight;

	// Load the vertex array with data.
	// First triangle.
	m_vertices[0].position = XMFLOAT3(left, top, m_z);		// Top left.
	m_vertices[1].position = XMFLOAT3(right, bottom, m_z);	// Bottom right.
	m_vertices[2].position = XMFLOAT3(left, bottom, m_z);	// Bottom left.

	// Second triangle.
	m_vertices[3].position = XMFLOAT3(left, top, m_z);		// Top left.
	m_vertices[4].position = XMFLOAT3(right, top, m_z);		// Top right.
	m_vertices[5].position = XMFLOAT3(right, bottom, m_z);	// Bottom right.
	
	// Lock the vertex buffer so it can be written to.
	result = m_devicecontext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotObtainImage2DBufferLock;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexType) * m_vertexCount));

	// Unlock the vertex buffer.
	m_devicecontext->Unmap(m_vertexBuffer, 0);

	return ErrorCodes::NoError;
}


void Image2D::RenderBuffers(void)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	m_devicecontext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	m_devicecontext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	m_devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


Result Image2D::LoadTexture(ID3D11Device* device, const char *filename)
{
	Result result;

	// Create the texture object.
	m_Texture = new Texture();
	if(!m_Texture)
	{
		return ErrorCodes::CouldNotAllocateImage2DTextureObject;
	}

	// Initialize the texture object.
	result = m_Texture->Initialise(filename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	return ErrorCodes::NoError;
}


void Image2D::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}
}
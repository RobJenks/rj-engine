#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "GameVarsExtern.h"
#include "Utility.h"
#include "DX11_Core.h"
#include "CoreEngine.h"
#include "Texture.h"

#include "Image2DRenderGroup.h"

Image2DRenderGroup::Image2DRenderGroup(void)
{
	m_instances = Image2DRenderGroup::InstanceCollection();
	m_device = NULL;
	m_devicecontext = NULL;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
	m_texturemode = Texture::APPLY_MODE::Normal;
	m_texturesize = INTVECTOR2(1, 1);
	m_ftexturesize = XMFLOAT2(1.0f, 1.0f);
	m_vertices = NULL;
	m_zorder = 0.0f;
	m_indexCount = 0;
	m_bufferinstances = 0;
	m_forcefullupdate = false;
	m_acceptsmouse = false;
}


Result Image2DRenderGroup::Initialize(Rendering::RenderDeviceType * device, int screenWidth, int screenHeight, const char *textureFilename, Texture::APPLY_MODE texturemode)
{ 
	Result result;

	// Store the device and screen details
	m_device = device;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_screenHalfWidth = (float)m_screenWidth / 2.0f;
	m_screenHalfHeight = (float)m_screenHeight / 2.0f;
	m_screenLeft = (m_screenHalfWidth * -1.0f);
	m_texturemode = texturemode;

	// Also get a reference to the device context
	m_devicecontext = Game::Engine->GetDeviceContext();

	// Initialize the vertex and index buffers.
	result = InitializeBuffers();
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Load the texture for this bitmap if a filename is specified.  If not, we can leave it as NULL for now
	if (textureFilename)
	{
		result = LoadTexture(device, textureFilename);
		if(result != ErrorCodes::NoError)
		{
			return result;
		}
	}

	// Return success
	return ErrorCodes::NoError;
}


Result Image2DRenderGroup::InitializeBuffers(void)
{
	INDEXFORMAT *indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// If we already have memory allocated (e.g. if we are here performing a full mid-render refresh) then deallocate it first
	if (m_vertices) { delete[] m_vertices; m_vertices = NULL; }

	// Set the number of vertices in the vertex array.  Six vertices are required per instance.
	int instancecount = (int)m_instances.size();
	m_vertexCount = instancecount * 6;
	
	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// If there are no instances then quit here as there is nothing to initialise
	if (instancecount == 0) return ErrorCodes::NoError;

	// Create the vertex array.
	m_vertices = new VertexType[m_vertexCount];
	if(!m_vertices)
	{
		return ErrorCodes::CouldNotAllocateImage2DRenderGroupVertexArray;
	}

	// Create the index array.
	indices = new INDEXFORMAT[m_indexCount];
	if(!indices)
	{
		return ErrorCodes::CouldNotAllocateImage2DRenderGroupIndexArray;
	}

	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// Load the vertex array with constant data, e.g. texture coords, that we do not need to set each frame
	int vertexindex = 0;
	float umax = 1.0f, vmax = 1.0f;
	for (int i = 0; i < instancecount; ++i)
	{
		// Calculate max texture coordinates based on the current texture mode
		if (m_texturemode == Texture::APPLY_MODE::Repeat) {
			if (m_instances[i].rotation == Rotation90Degree::Rotate0 || m_instances[i].rotation == Rotation90Degree::Rotate180) {
				umax = ((float)m_instances[i].size.x / m_ftexturesize.x);
				vmax = ((float)m_instances[i].size.y / m_ftexturesize.y);
			} else {
				umax = ((float)m_instances[i].size.y / m_ftexturesize.x);
				vmax = ((float)m_instances[i].size.x / m_ftexturesize.y);
			}
		}
		else { umax = 1.0f; vmax = 1.0f; }

		// Now map texture coordinates onto the vertices
		UpdateVertexTextureMappingCoords(&m_vertices[vertexindex], m_instances[i].rotation, umax, vmax);

		// Increment the vertex pointer 
		vertexindex += 6;
	}

	// Load the index array with data.
	for(int i = 0; i < m_indexCount; ++i) indices[i] = i;

	// Record the new size of the vertex buffers
	m_bufferinstances = instancecount;

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
    result = m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateImage2DRenderGroupVertexBuffer;
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
	result = m_device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotCreateImage2DRenderGroupIndexBuffer;
	}

	// Release the index array now that the buffer has been created and loaded.
	delete [] indices;
	indices = 0;

	return ErrorCodes::NoError;
}

Result Image2DRenderGroup::LoadTexture(Rendering::RenderDeviceType * device, const char *filename)
{
	Result result;

	// Create the texture object.
	m_Texture = new Texture();
	if(!m_Texture)
	{
		return ErrorCodes::CouldNotAllocateImage2DRenderGroupTextureObject;
	}

	// Initialize the texture object.
	result = m_Texture->Initialise(filename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Also store the texture size locally, for efficiency
	m_texturesize = m_Texture->GetTextureSize();
	m_ftexturesize = XMFLOAT2((float)m_texturesize.x, (float)m_texturesize.y);

	// Return success
	return ErrorCodes::NoError;
}

void Image2DRenderGroup::SetTextureMode(Texture::APPLY_MODE mode)
{
	// Store the new texture mode
	m_texturemode = mode;

	// Request a full update of the vertex buffers since the texture mode impacts mapping of texture coordinates within the buffer
	m_forcefullupdate = true;
}

void Image2DRenderGroup::SetTextureDirect(Texture *tex)
{
	// Dispose of the old texture, if we had one
	if (m_Texture) m_Texture->Shutdown();

	// Store a reference to the new texture
	m_Texture = tex;
	
	// Also store the texture size locally, for efficiency
	m_texturesize = m_Texture->GetTextureSize();
	m_ftexturesize = XMFLOAT2((float)m_texturesize.x, (float)m_texturesize.y);
}

void Image2DRenderGroup::Render(void)
{
	// If there are no instances in the collection then we have nothing to render
	int numinstances = (int)m_instances.size();
	if (numinstances == 0) return;

	// If the instance collection has changed in size since the vertex buffers were last constructed
	// then we need to reconstruct them to the new size
	if (numinstances != m_bufferinstances || m_forcefullupdate) InitializeBuffers();

	// Re-build the dynamic vertex buffer for rendering to potentially different locations on the screen.
	Result result = UpdateBuffers();
	if(result != ErrorCodes::NoError) return;

	// Reset the force-update flag, if it was set
	m_forcefullupdate = false;

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();
}


Result Image2DRenderGroup::UpdateBuffers(void)
{
	float left, right, top, bottom, zorder;
	float umax, vmax;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;

	// Precalculate fields where possible
	bool texturemoderepeat = (m_texturemode == Texture::APPLY_MODE::Repeat);

	// Look at each instance in turn
	int vertexindex = 0;
	bool bufferupdaterequired = false;
	for (int i=0; i<m_bufferinstances; i++)
	{
		// If the position & size of this instance has not changed then don't update anything since it
		// currently has the correct parameters.  Only exception is if the force update flag is set
		if( !( m_forcefullupdate || m_instances[i].HasChanged() ) )
		{
			// Manuall increment the vertex index to account for the fact that we are not rendering this element
			vertexindex += 6;

			// Skip directly to the next element
			continue;
		}
		
		// Record the fact that at least one instance has changed
		bufferupdaterequired = true;

		// Determine whether we need to remap texture coordinates based on a change in instance size
		bool textureremaprequired = (texturemoderepeat && ( (m_instances[i].size.x != m_instances[i].__prevsize.x) || 
															(m_instances[i].size.y != m_instances[i].__prevsize.y)));

		// Before updating the control fields, see if we need to update texture mapping coords due to a resize
		if (textureremaprequired)
		{
			// Calculate new texture mapping coordinates, if we are using a 'Repeat' texture mode and have resized this instance
			if (m_instances[i].rotation == Rotation90Degree::Rotate0 || m_instances[i].rotation == Rotation90Degree::Rotate180) {
				umax = ((float)m_instances[i].size.x / m_ftexturesize.x);
				vmax = ((float)m_instances[i].size.y / m_ftexturesize.y);
			} else {
				umax = ((float)m_instances[i].size.y / m_ftexturesize.x);
				vmax = ((float)m_instances[i].size.x / m_ftexturesize.y);
			}
		}
		else { umax = 1.0f; vmax = 1.0f; }

		// Update the texture mapping coords if (1) we have a resize-remap (as above), or (2) the instance has been rotated
		if (  textureremaprequired || (m_instances[i].rotation != m_instances[i].__prevrotation))
		{
			// Apply the new texture coordinates to this instance
			UpdateVertexTextureMappingCoords(&m_vertices[vertexindex], m_instances[i].rotation, umax, vmax);
		}

		// If the instance *has* changed then we update its control fields now  
		m_instances[i].UpdateControlFields();

		// If this instance is set to be rendered then calculate the position now
		if (m_instances[i].render)
		{
			// Calculate the screen coordinates of the left side of the bitmap.
			left = m_screenLeft + (float)m_instances[i].position.x;

			// Calculate the screen coordinates of the right side of the bitmap.
			right = left + (float)m_instances[i].size.x;

			// Calculate the screen coordinates of the top of the bitmap.
			top = m_screenHalfHeight - (float)m_instances[i].position.y;

			// Calculate the screen coordinates of the bottom of the bitmap.
			bottom = top - (float)m_instances[i].size.y;
		}
		else
		{
			// If not set to be rendered, position this item so that it is not visible
			left = top = -999.0f; right = bottom = -998.0f;
		}

		// Retrieve the z order index as well
		zorder = m_instances[i].zorder;

		// Load the vertex array with data.
		// First triangle.
		m_vertices[vertexindex].position = XMFLOAT3(left, top, zorder);		vertexindex++;		// Top left.
		m_vertices[vertexindex].position = XMFLOAT3(right, bottom, zorder);	vertexindex++;		// Bottom right.
		m_vertices[vertexindex].position = XMFLOAT3(left, bottom, zorder);	vertexindex++;		// Bottom left.

		// Second triangle.
		m_vertices[vertexindex].position = XMFLOAT3(left, top, zorder);		vertexindex++;		// Top left.
		m_vertices[vertexindex].position = XMFLOAT3(right, top, zorder);		vertexindex++;		// Top right.
		m_vertices[vertexindex].position = XMFLOAT3(right, bottom, zorder);	vertexindex++;		// Bottom right.
	}

	// If no instances were modified then we do not need to update the buffer and can return now
	if (!bufferupdaterequired) return ErrorCodes::NoError;

	// Lock the vertex buffer so it can be written to, now that all vertex data has been updated
	result = m_devicecontext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotObtainImage2DRenderGroupBufferLock;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexType) * m_vertexCount));

	// Unlock the vertex buffer.
	m_devicecontext->Unmap(m_vertexBuffer, 0);

	return ErrorCodes::NoError;
}

void Image2DRenderGroup::RenderBuffers(void)
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

void Image2DRenderGroup::Shutdown()
{
	// Release each instance within the render group in turn
	ReleaseAllInstances();

	// Release the bitmap texture.
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();
}

void Image2DRenderGroup::ReleaseAllInstances()
{
	// Loop through each instance in turn
	InstanceCollection::size_type n = m_instances.size();
	for (InstanceCollection::size_type i = 0; i < n; ++i)
	{
		// Call the instance shutdown method 
		m_instances[i].Shutdown();
	}

	// Clear the instance vector to release all allocated memory
	m_instances.clear();
}

void Image2DRenderGroup::ShutdownBuffers()
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

void Image2DRenderGroup::ReleaseTexture(void)
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}
}


Image2DRenderGroup::Instance *Image2DRenderGroup::AddInstance(INTVECTOR2 pos, float zorder, INTVECTOR2 size, bool render, Rotation90Degree rotation)
{
	// Create a new instance object and push onto the instance vector
	m_instances.push_back(Image2DRenderGroup::Instance(pos, zorder, size, render, rotation));

	// Now return a pointer to this new element in the vector
	return &(m_instances.at(m_instances.size()-1));	
}

// Removes an instance from the instance collection.  NOTE: this has undefined behaviour if 
// the instance in question doesn't already exist in the collection
void Image2DRenderGroup::RemoveInstance(Instance *instance)
{
	InstanceCollection::size_type size = m_instances.size();

	// If we have zero instances (nothing to remove) then return now
	if (size == 0) return;

	// If we have more than one instance then we need to swap this instance with the end one
	Image2DRenderGroup::Instance *lastinstance = &(m_instances[size - 1]);
	if (size > 1) std::swap(instance, lastinstance);
	
	// Now pop the last element off the vector to remove it from the collection
	m_instances.pop_back();
}

void Image2DRenderGroup::RemoveInstance(InstanceCollection::size_type index)
{
	InstanceCollection::size_type size = m_instances.size();

	// Make sure this index is valid
	if (index >= size) return;

	// If this instance is not already the last one then move it to the end now
	if (index != (size-1))
	{
		Image2DRenderGroup::Instance *instance = &(m_instances[index]);
		Image2DRenderGroup::Instance *lastinstance = &(m_instances[size - 1]);
		std::swap(instance, lastinstance);
	}

	// Now pop this last element from the back of the collection
	m_instances.pop_back();
}

Image2DRenderGroup::Instance * Image2DRenderGroup::GetInstanceByCode(std::string code)
{
	// Loop through the collection to find a matching code
	InstanceCollection::size_type n = m_instances.size();
	for (InstanceCollection::size_type i = 0; i < n; ++i)
	{
		if (m_instances[i].code == code) return &(m_instances[i]);
	}

	// Return NULL if there is no match
	return NULL;
}

Image2DRenderGroup::InstanceReference Image2DRenderGroup::GetInstanceReferenceByCode(std::string code)
{
	// Loop through the collection to find a matching code
	InstanceCollection::size_type n = m_instances.size();
	for (InstanceCollection::size_type i = 0; i < n; ++i)
	{
		if (m_instances[i].code == code)
		{
			// If we have a match, build a new instance reference object and return it
			return Image2DRenderGroup::InstanceReference(&(m_instances[i]), this, (int)i, code);
		}
	}

	// Otherwise return a null instance reference
	return Image2DRenderGroup::InstanceReference();
}

void Image2DRenderGroup::UpdateVertexTextureMappingCoords(Image2DRenderGroup::VertexType *v, Rotation90Degree rotation, float umax, float vmax)
{
	// Apply different texture coords depending on the rotation of this instance (allowed in 90-degree increments)
	switch (rotation)
	{
		case Rotation90Degree::Rotate90:
			(v+0)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			(v+1)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			(v+2)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			(v+3)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			(v+4)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			(v+5)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			return;

		case Rotation90Degree::Rotate180:
			(v+0)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			(v+1)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			(v+2)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			(v+3)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			(v+4)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			(v+5)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			return;

		case Rotation90Degree::Rotate270:
			(v+0)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			(v+1)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			(v+2)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			(v+3)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			(v+4)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			(v+5)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			return;

		default:	// i.e. zero degrees of rotation
			(v+0)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			(v+1)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			(v+2)->texture = XMFLOAT2(0.0f, vmax);			// Bottom left
			(v+3)->texture = XMFLOAT2(0.0f, 0.0f);			// Top left
			(v+4)->texture = XMFLOAT2(umax, 0.0f);			// Top right
			(v+5)->texture = XMFLOAT2(umax, vmax);			// Bottom right
			return;
	}
}


Image2DRenderGroup::~Image2DRenderGroup(void)
{
}

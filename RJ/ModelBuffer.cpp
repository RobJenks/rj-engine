#include "Utility.h"
#include "Texture.h"
#include "Material.h"
#include "ModelBuffer.h"


// Default constructor
ModelBuffer::ModelBuffer(void)
{
	// Default all values
	VertexBuffer = NULL;
	IndexBuffer = NULL;
	m_texture = NULL;
	m_vertexcount = m_indexcount = 0U;
	m_vertexsize = m_indexsize = 0U;
	m_materials = NULL;
	m_material_count = 0U;
}

// Initialise the buffers based on the supplied model data
Result ModelBuffer::Initialise(	ID3D11Device *device, const void **ppVertexdata, unsigned int vertexsize, unsigned int vertexcount,
								const void **ppIndexdata, unsigned int indexsize, unsigned int indexcount)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Parameter check
	if (!device || !ppVertexdata || !(*ppVertexdata) || !ppIndexdata || !(*ppIndexdata) || 
		vertexsize <= 0U || indexsize <= 0U || vertexcount <= 0U || indexcount <= 0U)
		return ErrorCodes::CannotInitialiseModelBufferWithInvalidData;

	// Store any parameters that we want to maintain within the object
	m_vertexcount = vertexcount;
	m_vertexsize = vertexsize;
	m_indexcount = indexcount;
	m_indexsize = indexsize;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = (m_vertexsize * m_vertexcount);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = (*ppVertexdata);
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &VertexBuffer);
	if (FAILED(hr))
	{
		return ErrorCodes::CouldNotCreateMBVertexBuffer;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = (m_indexsize * m_indexcount);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = (*ppIndexdata);
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &IndexBuffer);
	if (FAILED(hr)) return ErrorCodes::CouldNotCreateMBIndexBuffer;

	// We have initialised all model buffer data so return success
	return ErrorCodes::NoError;
}

// Sets the texture for this object
Result ModelBuffer::SetTexture(const char *filename)
{
	// We want to deallocate any texture that already exists
	if (m_texture)
	{
		m_texture->Shutdown();
		SafeDelete(m_texture);
	}

	// Now load the model texture, if required (or leave as NULL if a null input was provided)
	if (filename)
	{
		// Create the texture object
		m_texture = new Texture();
		if (!m_texture) return ErrorCodes::CouldNotCreateTextureObject;

		// Initialise the texture object
		Result result = m_texture->Initialise(filename);
		if (result != ErrorCodes::NoError) return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

// Sets the texture for this object
Result ModelBuffer::SetTexture(Texture *texture)
{
	// We want to deallocate any texture that already exists
	if (m_texture)
	{
		m_texture->Shutdown();
		SafeDelete(m_texture);
	}

	// Now simply store the texture object
	m_texture = texture;

	// Return success
	return ErrorCodes::NoError;
}


// Set the number of materials in this model and allocate space accordingly
void ModelBuffer::SetMaterialCount(unsigned int count)
{
	// Deallocate any existing material storage, if applicable
	if (m_materials != NULL)
	{
		SafeDeleteArray(m_materials);
		m_material_count = 0U;
	}

	// Store the new material count
	m_material_count = count;

	// Allocate space for the materials
	m_materials = new Material[m_material_count];
}

// Set details for the specified material
void ModelBuffer::SetMaterial(unsigned int index, const Material & material)
{
	// Parameter check
	if (index < 0 || index >= m_material_count) return;

	// Copy the material data
	m_materials[index] = material;

	// Also ensure that the material ID is set accordingly
	m_materials[index].Data.ID = index;
}


// Releases buffer resources (VB, IB) and initialises back to initial state.  Not required in normal use since this will be
// handled automatically when the object is deallocated
void ModelBuffer::ReleaseModelBufferResources(void)
{
	// Release each of the buffers if they have been allocated
	ReleaseIfExists(VertexBuffer);
	ReleaseIfExists(IndexBuffer);

	// Initialise other fields back to initial states
	m_vertexcount = m_indexcount = 0U;
	m_vertexsize = m_indexsize = 0U;
}

// Releases all resources and initialises back to initial state.  Includes model buffer resources (as per ReleaseModelBufferResources)
// as well as e.g. texture resources.  Not required in normal use since this will be handled automatically when the object is deallocated
void ModelBuffer::ReleaseAllResources(void)
{
	// Release all the resources allocated to model (VB / IB) buffers
	ReleaseModelBufferResources();
	
	// Deallocate the texture object if relevant
	if (m_texture)
	{
		m_texture->Shutdown();
		SafeDelete(m_texture);
	}

	// Deallocate material data if relevant
	if (m_materials)
	{
		SafeDeleteArray(m_materials);
	}
}

// Default destructor
ModelBuffer::~ModelBuffer(void)
{
	// Release all buffer resources
	ReleaseAllResources();
}




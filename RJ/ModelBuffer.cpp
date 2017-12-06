#include "Utility.h"
#include "Texture.h"
#include "Material.h"
#include "ModelBuffer.h"

// Indicates that the model buffer does not currently have an assigned per-frame render slot
const size_t ModelBuffer::NO_RENDER_SLOT = ((size_t)0U - (size_t)1U);

// Default constructor
ModelBuffer::ModelBuffer(void)
	:
	Material(NULL)
{
	ClearAllRenderSlots();
}

// Constructor to build a new buffer from the provided data
ModelBuffer::ModelBuffer(	const void **ppVertexdata, unsigned int vertexsize, unsigned int vertexcount,
							const void **ppIndexdata, unsigned int indexsize, unsigned int indexcount, const MaterialDX11 * material)
	:
	VertexBuffer(*ppVertexdata, vertexcount, vertexsize), 
	IndexBuffer(*ppIndexdata, indexcount, indexsize), 
	Material(material)
{
	ClearAllRenderSlots();
}

// Constructor to build a new buffer from existing buffer data that will be MOVED into the buffer
ModelBuffer::ModelBuffer(VertexBufferDX11 && vertex_buffer, IndexBufferDX11 && index_buffer, const MaterialDX11 * material)
	:
	VertexBuffer(std::move(vertex_buffer)), 
	IndexBuffer(std::move(index_buffer)), 
	Material(material)
{
	ClearAllRenderSlots();
}

// Constructor to build a new buffer from the provided data.  Index buffer will be automatically constructed as a sequential
// buffer matching the vertex buffer length, using the standard index format
ModelBuffer::ModelBuffer(const void **ppVertexdata, unsigned int vertexsize, unsigned int vertexcount, const MaterialDX11 * material)
	:
	VertexBuffer(*ppVertexdata, vertexsize, vertexcount), 
	IndexBuffer(vertexcount), 
	Material(material)
{
	ClearAllRenderSlots();
}


// Default destructor
ModelBuffer::~ModelBuffer(void)
{
}




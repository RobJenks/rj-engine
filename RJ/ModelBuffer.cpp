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


// Default destructor
ModelBuffer::~ModelBuffer(void)
{
}




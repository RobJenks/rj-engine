#pragma once

#ifndef __ModelBufferH__
#define __ModelBufferH__

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "DX11_Core.h"
#include "Rendering.h"
#include "RenderQueueShaders.h"
#include "MaterialDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"

// This class has no special alignment requirements
// Model buffer encapsulates a { VB, IB, Material } with a few other properties.  It is intended to be entirely sufficient for rendering
// of a mesh with all associated effects.  If a model requires multiple materials then it should contain multiple ModelBuffer objects, 
// each of which is rendered sequentially, and where the VB/IB include only relevant vertices for each (or at minimum, IB references
// only valid vertices from the full VB duplicated in each)
class ModelBuffer
{
public:

	// Indicates that the model buffer does not currently have an assigned per-frame render slot
	static const size_t				NO_RENDER_SLOT;

	// Default constructor
	ModelBuffer(void);

	// Constructor to build a new buffer from the provided data
	ModelBuffer(const void **ppVertexdata, unsigned int vertexsize, unsigned int vertexcount,
				const void **ppIndexdata, unsigned int indexsize, unsigned int indexcount, const MaterialDX11 * material);

	
	// Vertex buffer
	VertexBufferDX11				VertexBuffer;

	// Index buffer
	IndexBufferDX11					IndexBuffer;

	// Material.  Pointer to material potentially shared across many models.  Model buffers only contain one material since, as per class
	// header comments above, an object requiring multiple materials should contain multiple ModelBuffers which are rendered sequentially
	const MaterialDX11 * 			Material;

	// Other buffer properties
	CMPINLINE std::string			GetCode(void) const					{ return m_code; }
	CMPINLINE void					SetCode(const std::string & code)	{ m_code = code; }
	


	// TODO: Probably need to remove this
	// Render queue slot assigned to this buffer for the current frame, or (0U-1) if none
	CMPINLINE size_t				GetAssignedRenderSlot(size_t shader) const		{ return m_render_slot[shader]; }
	CMPINLINE bool					HasAssignedRenderSlot(size_t shader) const		{ return (m_render_slot[shader] != ModelBuffer::NO_RENDER_SLOT); }
	CMPINLINE void					AssignRenderSlot(size_t shader, size_t slot)	{ m_render_slot[shader] = slot; }
	CMPINLINE void					ClearRenderSlot(size_t shader)					{ m_render_slot[shader] = ModelBuffer::NO_RENDER_SLOT; }
	CMPINLINE void					ClearAllRenderSlots(void) 
	{
		for (size_t shader = 0U; shader < (size_t)RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++shader) m_render_slot[shader] = ModelBuffer::NO_RENDER_SLOT;
	}

	// Default destructor
	~ModelBuffer(void);


protected:

	std::string				m_code;

	// Render queue slot assigned to this buffer for the current frame, or (0U-1) if none
	size_t					m_render_slot[RenderQueueShader::RM_RENDERQUEUESHADERCOUNT];		
};




#endif








